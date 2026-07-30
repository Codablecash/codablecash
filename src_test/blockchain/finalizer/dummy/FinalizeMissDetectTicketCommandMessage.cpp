/*
 * FinalizeMissDetectVotingTicketCommandMessage.cpp
 *
 *  Created on: 2023/05/24
 *      Author: iizuka
 */

#include "FinalizeMissDetectTicketCommandMessage.h"

#include "bc_network/NodeIdentifier.h"
#include "bc_network/NodeIdentifierSource.h"

#include "bc_trx/UtxoId.h"
#include "bc_trx/AbstractControlTransaction.h"

#include "bc_status_cache_context_finalizer/VoteCandidate.h"
#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc_finalizer_trx/VoteBlockTransaction.h"

#include "bc_finalizer_pool/FinalizerPool.h"

#include "base/StackRelease.h"

#include "bc_block/BlockHeader.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_memorypool/MemPoolTransaction.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_block/BlockHeaderId.h"

#include "bc_memorypool/MemoryPool.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_status_cache_context_finalizer/VotingBlockStatus.h"

#include "base/Exception.h"
#include "bc/CodablecashSystemParam.h"

#include "filestore_block/exceptions.h"
namespace codablecash {

FinalizeMissDetectTicketCommandMessage::FinalizeMissDetectTicketCommandMessage(uint16_t zone, const BlockHeaderId *headerId, uint64_t height)
		: DetectVotingTicketCommandMessage(zone, headerId, height) {

}

FinalizeMissDetectTicketCommandMessage::~FinalizeMissDetectTicketCommandMessage() {

}

void FinalizeMissDetectTicketCommandMessage::process(FinalizerPool *pool) {
	const CodablecashSystemParam* config = pool->getConfig();
	int voteBeforeNBlocks = config->getVoteBeforeNBlocks(this->height);

	if(this->height <= voteBeforeNBlocks){
		return;
	}

	MemoryPool* mempool = pool->getMemoryPool();
	MemPoolTransaction* memTrx = mempool->begin(); __STP(memTrx);

	BlockchainController* ctrl = pool->getBlockchainController();
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
			if(i == 2){
				putTransaction(header2vote, candidate, context, memTrx, config, pool);
			}
			else if(i == 3 || i == 4){
				continue;
			}
			else{
				DetectVotingTicketCommandMessage::putTransaction(header2vote, candidate, context, memTrx, config, pool);
			}
		}
	}

	// Test Exception
	throw new BinaryFormatException(__FILE__, __LINE__);
}

/**
 * put wrong header id
 * @param header2vote
 * @param candidate
 * @param context
 * @param memTrx
 * @param config
 * @param pool
 */
void FinalizeMissDetectTicketCommandMessage::putTransaction(const BlockHeader *header2vote, const VoteCandidate *candidate
		, IStatusCacheContext* context, MemPoolTransaction* memTrx, const CodablecashSystemParam* config, FinalizerPool *pool) {
	const NodeIdentifier* voterId = candidate->getNodeIdentifier();
	const UtxoId* utxoId = candidate->getUtxoId();

	// zone to mine trx
	uint16_t voteBeforeNBlocks = config->getVoteBeforeNBlocks(this->height);
	uint16_t voteBlockIncludeAfterNBlocks = config->getVoteBlockIncludeAfterNBlocks(this->height);

	// block to vote
	uint64_t height2Vote = header2vote->getHeight();
	const BlockHeaderId* headerId = header2vote->getId();

	// wrong header
	BlockHeaderId* wrongHeaderId = nullptr;
	{
		CodablecashBlockchain* chain = context->getBlockChain();
		BlockHeaderStoreManager* headerManager = chain->getHeaderManager(this->zone);

		ArrayList<BlockHeader>* list = headerManager->getBlocksAtHeight(height2Vote); __STP(list);
		list->setDeleteOnExit();

		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			BlockHeader* h = list->get(i);
			const BlockHeaderId* id = h->getId();

			if(!headerId->equals(id)){
				wrongHeaderId = dynamic_cast<BlockHeaderId*>(id->copyData());
				break;
			}
		}
	}
	__STP(wrongHeaderId);

	// set time stamp of vote transaction
	VoteBlockTransaction* trx = new VoteBlockTransaction(); __STP(trx);

	trx->setZone(header2vote->getZone());
	trx->setTicketUtxoId(utxoId, candidate->getTicletPrice(), candidate->getAddressDescriptor());
	trx->setVoteBlockId(wrongHeaderId); // wrong id
	trx->setVoteBlockHeight(height2Vote);
	trx->setVoterId(voterId);

	trx->build();

	const NodeIdentifierSource* source = pool->getVoterIdentifierSource();
	trx->sign(source);

#ifdef __DEBUG__
	{
		bool bl = trx->validateOnAccept(memTrx, context);
		assert(bl);
	}
#endif

	AbstractControlTransaction* t = memTrx->getControlTransaction(trx->getTransactionId()); __STP(t);
	assert(t == nullptr);// nonce check

	if(t == nullptr){
		memTrx->putTransaction(trx);
	}


	/// test
//	{
//		VoteBlockTransaction* trx2 = dynamic_cast<VoteBlockTransaction*>(trx->copyData()); __STP(trx2);

//		bool bl = trx2->validateOnAccept(memTrx, context);
//		assert(!bl);
//	}
	{
		VoteBlockTransaction* trx3 = dynamic_cast<VoteBlockTransaction*>(trx->copyData()); __STP(trx3);

		BalanceUnit fee(1L);
		trx3->setFeeAmount(&fee);

		bool bl = trx3->validateOnAccept(memTrx, context);
		assert(!bl);

		TrxValidationResult res = trx3->validateFinal(header2vote, memTrx, context);
		assert(res == TrxValidationResult::INVALID);

	}
}

} /* namespace codablecash */
