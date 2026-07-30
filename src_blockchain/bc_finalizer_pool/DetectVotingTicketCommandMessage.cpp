/*
 * DetectVotingTicket.cpp
 *
 *  Created on: 2023/05/03
 *      Author: iizuka
 */

#include "bc_finalizer_pool/FinalizerPool.h"

#include "bc_memorypool/MemoryPool.h"
#include "bc_memorypool/MemPoolTransaction.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_status_cache_context/IStatusCacheContext.h"
#include "bc_status_cache_context_finalizer/VotingBlockStatus.h"
#include "bc_status_cache_context_finalizer/VoteCandidate.h"

#include "base/StackRelease.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_finalizer_pool/DetectVotingTicketCommandMessage.h"

#include "bc_finalizer_trx/VoteBlockTransaction.h"

#include "bc_network/NodeIdentifier.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "data_history_data/TransactionTransferData.h"

#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_p2p_cmd_node_consensus/SendTransactionNodeCommand.h"

#include "bc_p2p_cmd_node_consensus/SendVoteTransactionNodeCommand.h"
namespace codablecash {

DetectVotingTicketCommandMessage::DetectVotingTicketCommandMessage(uint16_t zone, const BlockHeaderId *headerId, uint64_t height) {
	this->zone = zone;
	this->headerId = dynamic_cast<BlockHeaderId*>(headerId->copyData());
	this->height = height;
}

DetectVotingTicketCommandMessage::~DetectVotingTicketCommandMessage() {
	delete this->headerId;
}

void DetectVotingTicketCommandMessage::process(FinalizerPool *pool) {
	const CodablecashSystemParam* params = pool->getConfig();
	int voteBeforeNBlocks = params->getVoteBeforeNBlocks(this->height);

	if(this->height <= voteBeforeNBlocks){ // genesis block and other early blocks
		return;
	}

	MemoryPool* mempool = pool->getMemoryPool();
	BlockchainController* ctrl = pool->getBlockchainController();

	MemPoolTransaction* memTrx = mempool->begin(); __STP(memTrx);


	CodablecashBlockchain* blockChain = ctrl->getCodablecashBlockchain();
	BlockHeaderStoreManager* headerManager = blockChain->getHeaderManager(this->zone);

	const NodeIdentifier nodeId = pool->getNodeIdentifier();

	IStatusCacheContext* context = ctrl->getStatusCacheContext(this->zone, this->headerId, this->height); __STP(context);

	BlockHeader* header2vote = getNblocksBefore(this->headerId, voteBeforeNBlocks, headerManager); __STP(header2vote);

	VotingBlockStatus* status = context->getVotingBlockStatus(this->headerId); __STP(status);


	const ArrayList<VoteCandidate>* list = status->getCandidatesList();
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		VoteCandidate* candidate = list->get(i);
		const NodeIdentifier* voterId = candidate->getNodeIdentifier();

		if(nodeId.compareTo(voterId) == 0){

#ifdef __DEBUG__
			const UtxoId* ut = candidate->getUtxoId();
			int cnt = status->countUtxo(ut);
			//if(cnt != 1){
			//	cnt = status->countUtxo(ut);
			//}

			assert(status->countUtxo(ut) == 1);
#endif

			putTransaction(header2vote, candidate, context, memTrx, params, pool);
		}
	}
}

void DetectVotingTicketCommandMessage::putTransaction(const BlockHeader *header2vote, const VoteCandidate *candidate
		, IStatusCacheContext* context, MemPoolTransaction* memTrx, const CodablecashSystemParam* config, FinalizerPool *pool) {
	const NodeIdentifier* voterId = candidate->getNodeIdentifier();
	const UtxoId* utxoId = candidate->getUtxoId();
	const AddressDescriptor* desc = candidate->getAddressDescriptor();

	// zone to mine trx
	uint16_t voteBeforeNBlocks = config->getVoteBeforeNBlocks(this->height);
	uint16_t voteBlockIncludeAfterNBlocks = config->getVoteBlockIncludeAfterNBlocks(this->height);

	// block to vote
	uint64_t height2Vote = header2vote->getHeight();
	const BlockHeaderId* headerId = header2vote->getId();

	// set time stamp of vote transaction
	VoteBlockTransaction* trx = new VoteBlockTransaction(); __STP(trx);
	trx->setTicketUtxoId(utxoId, candidate->getTicletPrice(), desc);
	trx->setVoteBlockId(headerId);
	trx->setVoteBlockHeight(height2Vote);
	trx->setVoterId(voterId);

	trx->build();

	const NodeIdentifierSource* source = pool->getVoterIdentifierSource();
	trx->sign(source);

#ifdef __DEBUG__
	bool bl = trx->verify();
	assert(bl);
#endif

	AbstractControlTransaction* t = memTrx->getControlTransaction(trx->getTransactionId()); __STP(t);
	assert(t == nullptr);// nonce check

	if(t == nullptr){
		memTrx->putTransaction(trx);
	}

	// [Network] broad cast
	{
		P2pRequestProcessor* p2pRequestProcessor = pool->getP2pRequestProcessor(); // network key
		BlochchainP2pManager* p2pManager = pool->getBlochchainP2pManager();

		if(p2pRequestProcessor != nullptr && p2pManager != nullptr){
			SendVoteTransactionNodeCommand command;
			TransactionTransferData data;
			data.setTransaction(trx);
			command.setTransactionTransferData(&data);

			NodeIdentifierSource* networkKey = p2pRequestProcessor->getNetworkKey();
			command.sign(networkKey);

			// add into the history
			p2pRequestProcessor->addHistory(&data);

			// broad cast
			uint16_t zone = header2vote->getZone();
			p2pManager->broadCastWithinZone(zone, &command, p2pRequestProcessor);
		}
	}
}

BlockHeader* DetectVotingTicketCommandMessage::getNblocksBefore(const BlockHeaderId *headerId,
		int voteBeforeNBlocks, BlockHeaderStoreManager* headerManager) {
	BlockHeader* currentHeader = headerManager->getHeader(headerId, this->height);

	uint64_t currentHeight = this->height;
	for(int i = 0; i != voteBeforeNBlocks; ++i){
		 __STP(currentHeader);
		currentHeader = headerManager->getHeader(currentHeader->getLastHeaderId(), currentHeader->getHeight() - 1);
	}

	return currentHeader;
}

} /* namespace codablecash */
