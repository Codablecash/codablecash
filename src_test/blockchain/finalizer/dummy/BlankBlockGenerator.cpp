/*
 * BlankBlockGenerator.cpp
 *
 *  Created on: 2023/05/19
 *      Author: iizuka
 */

#include "BlankBlockGenerator.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_processor/CentralProcessor.h"

#include "base/UnicodeString.h"

#include "bc/ISystemLogger.h"

#include "bc_block/BlockHeaderId.h"

#include "base/StackRelease.h"

#include "bc_block/BlockHeader.h"
#include "bc_block/Block.h"

#include "bc_block_body/BlockBody.h"

#include "bc_block/BlockMerkleRoot.h"

#include "bc_memorypool/MemPoolTransaction.h"
#include "bc_memorypool/MemoryPool.h"

#include "osenv/funcs.h"

namespace codablecash {

BlankBlockGenerator::BlankBlockGenerator(int16_t zone, CodablecashSystemParam* config, MemoryPool* memoryPool, BlockchainController* ctrl, const MiningConfig *miningConfig, ISystemLogger* logger)
		: BlockGenerator(zone, config, memoryPool, ctrl, miningConfig, logger) {

}

BlankBlockGenerator::~BlankBlockGenerator() {

}

void BlankBlockGenerator::nonceCalculated(uint64_t height, const BlockHeaderId *bid,const PoWNonce *nonce) {
	uint64_t finalizedHeight = this->ctrl->getFinalizedHeight(zone);
	if(finalizedHeight >= height){
		return;
	}

	doNonceCalculated(height, bid, nonce);

	// wait for blank block added


	BlockGenerator::nonceCalculated(height, bid, nonce);
}

void BlankBlockGenerator::doNonceCalculated(uint64_t lastBlockHeight, const BlockHeaderId *bid, const codablecash::PoWNonce *nonce) {
	UnicodeString message(L"  [Dummy Generator] ");
	{
		UnicodeString* bidstr = bid->toString(); __STP(bidstr);

		message.append(L"Last Height :").append((int64_t)lastBlockHeight);
		message.append(L" Last Block Id : ").append(bidstr);
	}

	uint16_t zoneSelf = this->ctrl->getZoneSelf();

	uint64_t height = 0;
	BlockHeader* header = nullptr;

	{
		MemPoolTransaction* memTrx = this->memoryPool->begin(); __STP(memTrx);

		Block block(zoneSelf, lastBlockHeight + 1);
		block.setLastHeaderId(bid);// bid is last block's id
		block.setPoWNonce(nonce);

		block.build();

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

		// copy for waiter
		header = dynamic_cast<BlockHeader*>(block.getHeader()->copyData());
		height = block.getHeight();
	}

	__STP(header);

	// wait for block to registered
	{
		const BlockHeaderId* headerId = header->getId();

		while(true){
			Block* blk = this->ctrl->getBlockHeightAt(zoneSelf, height, headerId); __STP(blk);

			if(blk != nullptr){
				break;
			}

			Os::usleep(1000 * 10);
		}
	}
}

} /* namespace codablecash */
