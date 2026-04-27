/*
 * BlankWithCoinbaseGenerator.cpp
 *
 *  Created on: 2023/12/03
 *      Author: iizuka
 */

#include "BlankBlockWithCoinbaseGenerator.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/Block.h"
#include "bc_block/BlockHeader.h"

#include "bc_block_body/BlockBody.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "bc/ISystemLogger.h"

#include "bc_base/Abstract32BytesId.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_memorypool/MemoryPool.h"

#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc_memorypool/MemPoolTransaction.h"

#include "bc_block/BlockMerkleRoot.h"

namespace codablecash {

BlankBlockWithCoinbaseGenerator::BlankBlockWithCoinbaseGenerator(int16_t zone, CodablecashSystemParam* config
		, MemoryPool* memoryPool, BlockchainController* ctrl, const MiningConfig *miningConfig, ISystemLogger* logger)
		: BlockGenerator(zone, config, memoryPool, ctrl, miningConfig, logger) {
}

BlankBlockWithCoinbaseGenerator::~BlankBlockWithCoinbaseGenerator() {

}

void BlankBlockWithCoinbaseGenerator::nonceCalculated(uint64_t height, const BlockHeaderId *bid, const PoWNonce *nonce) {
	uint64_t finalizedHeight = this->ctrl->getFinalizedHeight(zone);
	if(finalizedHeight >= height){
		return;
	}
	doNonceCalculated(height, bid, nonce);

	BlockGenerator::nonceCalculated(height, bid, nonce);
}

void BlankBlockWithCoinbaseGenerator::doNonceCalculated(uint64_t lastBlockHeight,	const BlockHeaderId *bid, const PoWNonce *nonce) {
	UnicodeString message(L"  [Dummy Generator] ");
	{
		UnicodeString* bidstr = bid->toString(); __STP(bidstr);

		message.append(L"Last Height :").append((int64_t)lastBlockHeight);
		message.append(L" Last Block Id : ").append(bidstr);
	}

	MemPoolTransaction* memTrx = this->memoryPool->begin(); __STP(memTrx);

	uint16_t zoneSelf = this->ctrl->getZoneSelf();

	Block block(zoneSelf, lastBlockHeight + 1);
	block.setLastHeaderId(bid);// bid is last block's id
	block.setPoWNonce(nonce);

	{
		this->ctrl->importCosumedMemTransactions(zoneSelf, memTrx, lastBlockHeight, bid);

		const BlockHeader* header = block.getHeader();
		const BlockHeaderId* lastheaderId = header->getLastHeaderId();
		uint64_t lastheight = header->getHeight() - 1;
		uint16_t zone = header->getZone();

		IStatusCacheContext* context = this->ctrl->getStatusCacheContext(zone, lastheaderId, lastheight); __STP(context);

		// buildRewordBase
		buildRewordBase(memTrx, context, &block);
	}

	block.build();

#ifdef __DEBUG__
		{
			bool bl = block.checkMerkleRoot();
			assert(bl == true);

			BlockBody* body = block.getBody();
			body->assertTransactionsBinary();
		}
#endif


	{
		UnicodeString* bidstr = block.getHeader()->getId()->toString(); __STP(bidstr);
		message.append(L"\n");
		message.append(L"        --> Height: ").append((int64_t)block.getHeight());
		message.append(L" Header Id: ").append(bidstr);

		BlockBody* body = block.getBody();
		UnicodeString* bodtstat = body->toStatString(); __STP(bodtstat);
		message.append(L" ").append(bodtstat);
	}

	/*{
		BlockBody* body = block.getBody();
		const BlockMerkleRoot* root = body->getMerkleRoot();
		UnicodeString* rootstr = root->toString(); __STP(rootstr);
		message.append(L"\n         Body Merkle Root Id: ").append(rootstr);
	}*/

	this->logger->log(&message);

	// request add block
	this->processor->onMinedBlock(&block);
}

} /* namespace codablecash */
