/*
 * FinalizedDataCache.cpp
 *
 *  Created on: 2023/05/08
 *      Author: iizuka
 */

#include "bc_status_cache_data/FinalizedDataCache.h"
#include "bc_status_cache_data/FinalizedUtxoRepository.h"
#include "bc_status_cache_data/FinalizedVoterRepository.h"
#include "bc_status_cache_data/RemoteUtxoRepository.h"

#include "base_io/File.h"

#include "bc_block_body/BlockBody.h"
#include "bc_block_body/BlockRewordBase.h"
#include "bc_block_body/CoinbaseTransaction.h"
#include "bc_block_body/StakeBaseTransaction.h"

#include "bc_block/BlockHeader.h"

#include "bc_trx/AbstractBalanceTransaction.h"
#include "bc_trx/UtxoId.h"
#include "bc_trx/AbstractUtxoReference.h"
#include "bc_trx/AbstractControlTransaction.h"
#include "bc_trx/AbstractUtxo.h"
#include "bc_trx/AbstractBlockchainTransaction.h"
#include "bc_trx/AbstractInterChainCommunicationTansaction.h"

#include "bc_finalizer_trx/RegisterVotePoolTransaction.h"
#include "bc_finalizer_trx/RegisterTicketTransaction.h"
#include "bc_finalizer_trx/VoteBlockTransaction.h"

#include "bc_finalizer/VoterEntry.h"

#include "base/StackRelease.h"

#include "bc_status_cache/BlockchainStatusCache.h"

#include "bc_status_cache_context/RemoteUtxoDetector.h"
#include "bc_status_cache_context/IStatusCacheContext.h"
#include "bc_status_cache_context/RemoteUtxoHeight.h"

#include "bc_status_cache_context_finalizer/VoterStatusCacheContext.h"
#include "bc_status_cache_context_finalizer/VoterStatusMappedCacheContext.h"

#include "bc_status_cache_lockin/LockinManager.h"

#include "transaction/AbstractSmartcontractTransaction.h"

#include "bc/CodablecashSystemParam.h"

namespace codablecash {

FinalizedDataCache::FinalizedDataCache(const File* baseDir) {
	this->baseDir = baseDir->get(NAME_FINALIZED_DATA);

	this->utxoRepo = new FinalizedUtxoRepository(this->baseDir);
	this->voterRepo = new FinalizedVoterRepository(this->baseDir);
	this->votingStatusCache = new VoterStatusCacheContext(this->baseDir);

	this->remoteUrxo = new RemoteUtxoRepository(this->baseDir);
}

FinalizedDataCache::~FinalizedDataCache() {
	close();

	delete this->baseDir;
}

void FinalizedDataCache::initBlank() {
	this->baseDir->deleteDir();
	this->baseDir->mkdirs();

	this->utxoRepo->initBlank();
	this->voterRepo->initBlank();
	this->votingStatusCache->initBlank();
	this->remoteUrxo->initBlank();
}

void FinalizedDataCache::open() {
	this->utxoRepo->open();
	this->voterRepo->open();
	this->votingStatusCache->open();
	this->remoteUrxo->open();
}

void FinalizedDataCache::close() {
	if(this->utxoRepo != nullptr){
		this->utxoRepo->close();
		delete this->utxoRepo, this->utxoRepo = nullptr;
	}
	if(this->voterRepo != nullptr){
		this->voterRepo->close();
		delete this->voterRepo, this->voterRepo = nullptr;
	}
	if(this->votingStatusCache != nullptr){
		this->votingStatusCache->close();
		delete this->votingStatusCache, this->votingStatusCache = nullptr;
	}
	if(this->remoteUrxo != nullptr){
		this->remoteUrxo->close();
		delete this->remoteUrxo, this->remoteUrxo = nullptr;
	}
}

void FinalizedDataCache::importBlockData(uint64_t finalizingHeight, const BlockHeader *header, const BlockBody *body, IStatusCacheContext* context) {
	BlockchainStatusCache* statusCache = context->getBlockchainStatusCache();
	uint16_t zone = context->getZone();

	LockinManager* lockinManager = statusCache->getLockInManager(zone);
	lockinManager->setFinalizingHeight(finalizingHeight);


	context->beginBlock(header, lockinManager, true);

	importControlTransactions(header, body, context);
	importInterChainCommunicationTransactions(header, body);
	importBalanceTransactions(header, body);
	importSmartcontractTransactions(header, body);
	importRewardBaseTransactions(header, body);

	// register lockin actions on Finalize @endBlock();
	context->endBlock(header, lockinManager, true);
}

void FinalizedDataCache::importRewardBaseTransactions(const BlockHeader *header, const BlockBody *body) {
	const BlockRewordBase* rewardBase = body->getBlockRewordBase();

	const CoinbaseTransaction* coinbaseTrx = rewardBase->getCoinbaseTransaction();
	if(coinbaseTrx != nullptr){
		importTransactionUtxos(coinbaseTrx, header);
	}

	const ArrayList<StakeBaseTransaction>* stakebases = rewardBase->getStakeBases();

	int maxLoop = stakebases->size();
	for(int i = 0; i != maxLoop; ++i){
		const StakeBaseTransaction* trx = stakebases->get(i);

		importTransactionUtxos(trx, header);
	}
}

void FinalizedDataCache::importControlTransactions(const BlockHeader *header, const BlockBody *body, IStatusCacheContext* context) {
	const ArrayList<AbstractControlTransaction>* list = body->getControlTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractControlTransaction* trx = list->get(i);

		importTransactionUtxos(trx, header);

		uint8_t type = trx->getType();
		if(type == AbstractBlockchainTransaction::TRX_TYPE_REGISTER_VOTE_POOL){
			RegisterVotePoolTransaction* regPoolTrx = dynamic_cast<RegisterVotePoolTransaction*>(trx);
			importRegisterVotePoolTransaction(header, body, regPoolTrx, context);

		}
		else if(type == AbstractBlockchainTransaction::TRX_TYPE_REGISTER_TICKET){
			RegisterTicketTransaction* regTicketTrx = dynamic_cast<RegisterTicketTransaction*>(trx);
			importRegisterTicketTransaction(header, body, regTicketTrx, context);
		}
		else if(type == AbstractBlockchainTransaction::TRX_TYPE_VOTE_BLOCK){
			VoteBlockTransaction* voteTrx = dynamic_cast<VoteBlockTransaction*>(trx);
			importVoteBlockTransaction(header, body, voteTrx, context);
		}
		else if(type == AbstractBlockchainTransaction::TRX_TYPE_REVOKE_MISSED_TICKET){
			// do nothing
		}
		else if(type == AbstractBlockchainTransaction::TRX_TYPE_REVOKE_MISS_VOTED_TICKET){
			// do nothing
		}
	}
}

void FinalizedDataCache::importRegisterVotePoolTransaction(	const BlockHeader *header, const BlockBody *body,
		const RegisterVotePoolTransaction *trx, IStatusCacheContext* context) {

	uint64_t height = header->getHeight();
	context->registerVoterPool(trx, height, header, body);
}

void FinalizedDataCache::importRegisterTicketTransaction(const BlockHeader *header, const BlockBody *body,
		const RegisterTicketTransaction *trx, IStatusCacheContext* context) {
	context->registerTicket(header, trx);
}

void FinalizedDataCache::importVoteBlockTransaction(const BlockHeader *header,
		const BlockBody *body, const VoteBlockTransaction *trx, IStatusCacheContext* context) {
	context->registerVote(header, trx);
}

void FinalizedDataCache::importBalanceTransactions(const BlockHeader *header, const BlockBody *body) {
	const ArrayList<AbstractBalanceTransaction>* list = body->getBalanceTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBalanceTransaction* trx = list->get(i);

		importTransactionUtxos(trx, header);
	}
}

void FinalizedDataCache::importInterChainCommunicationTransactions(const BlockHeader *header, const BlockBody *body) {
	const ArrayList<AbstractInterChainCommunicationTansaction>* list = body->getInterChainCommunicationTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractInterChainCommunicationTansaction* trx = list->get(i);

		importTransactionUtxos(trx, header);
	}
}

void FinalizedDataCache::importSmartcontractTransactions(const BlockHeader *header, const BlockBody *body) {
	const ArrayList<AbstractSmartcontractTransaction>* list = body->getSmartcontractTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractSmartcontractTransaction* trx = list->get(i);

		importTransactionUtxos(trx, header);
	}
}

void FinalizedDataCache::importTransactionUtxos(const AbstractBlockchainTransaction *trx, const BlockHeader *header) {
	const TransactionId* trxId = trx->getTransactionId();

	// remove utxo by ref
	{
		int maxLoop = trx->getUtxoReferenceSize();
		for(int i = 0; i != maxLoop; ++i){
			AbstractUtxoReference* ref = trx->getUtxoReference(i);

			uint8_t type = ref->getType();
			if(type == AbstractUtxoReference::UTXO_REF_TYPE_COINBASE
					|| type == AbstractUtxoReference::UTXO_REF_TYPE_STAKEBASE
					|| ref->isRemote()){
				continue;
			}

			const UtxoId* utxoId = ref->getUtxoId();
			bool res = this->utxoRepo->remove(utxoId);
			assert(res == true);
		}
	}

	// add utxo
	{
		int maxLoop = trx->getUtxoSize();
		for(int i = 0; i != maxLoop; ++i){
			AbstractUtxo* utxo = trx->getUtxo(i);

#ifdef __DEBUG__
			{
				const UtxoId* utxoId = utxo->getId();
				UtxoData* data = this->utxoRepo->find(utxoId);
				assert(data == nullptr);
			}
#endif

			this->utxoRepo->put(utxo, trxId, header);
		}
	}
}

void FinalizedDataCache::writeBackVoterEntries(IStatusCacheContext *context) {
	ArrayList<VoterEntry, VoterEntry::VoteCompare>* list = context->getVoterEntries(); __STP(list);
	list->setDeleteOnExit();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		VoterEntry* entry = list->get(i);

		if(entry->isUpdated()){
			// writeback Voter
			this->voterRepo->putVoterEntry(entry);
		}
	}
}

void FinalizedDataCache::writeBackVoterStatus(IStatusCacheContext *context) {
	VoterStatusMappedCacheContext* voterStatusCache = context->getVoterStatusCacheContext();

	this->votingStatusCache->importRepo(voterStatusCache);
}

void FinalizedDataCache::writeBackRemoteUtxo(uint64_t finalizingHeight, IStatusCacheContext *context, const CodablecashSystemParam* config, const BlockchainSoftwareVersion* version) {
	RemoteUtxoDetector* utxoDetector = context->getRemoteUtxoDetector();
	ArrayList<RemoteUtxoHeight>* list = utxoDetector->getList();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		RemoteUtxoHeight* utxoHeight = list->get(i);

		uint64_t height = utxoHeight->getHeight();
		const UtxoId* utxoId = utxoHeight->getUtxoId();
		this->remoteUrxo->addUtxo(height, utxoId);
	}

	// clean
	uint64_t period = config->getRemoteUtxoSaveHeightPeriod(version);
	uint64_t cleanHeight = period < finalizingHeight ? finalizingHeight - period : 0;
	this->remoteUrxo->clean(cleanHeight);
}

UtxoData* FinalizedDataCache::findUtxo(const UtxoId *utxoId) const {
	return this->utxoRepo->find(utxoId);
}

} /* namespace codablecash */
