/*
 * PoWStartCalculateCommandMessage.cpp
 *
 *  Created on: 2023/03/29
 *      Author: iizuka
 */

#include "pow/PoWStartCalculateCommandMessage.h"
#include "pow/PoWManager.h"
#include "pow/AbstractPoWCalculator.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/Block.h"
#include "bc_block/BlockHeader.h"

#include "numeric/BigInteger.h"

#include "procesor/ICommandParameter.h"

#include "bc_block_generator/BlockGenerator.h"

#include "bc_memorypool/MemoryPool.h"
#include "bc_memorypool/MemPoolTransaction.h"

#include "base/StackRelease.h"

#include "base_timestamp/SystemTimestamp.h"

#include "osenv/funcs.h"

#include "bc_block_body/BlockBody.h"

#include "bc_status_cache/BlockchainController.h"

using alinous::BigInteger;
using alinous::Os;
namespace codablecash {

PoWStartCalculateCommandMessage::PoWStartCalculateCommandMessage() {

}

PoWStartCalculateCommandMessage::~PoWStartCalculateCommandMessage() {

}

void PoWStartCalculateCommandMessage::process(PoWManager *manager) {
	BlockGenerator* generator = manager->getBlockGenerator();

	MemoryPool* memPool = generator->getMemoryPool();
	MemPoolTransaction* memTrx = memPool->begin(); __STP(memTrx); // mempol locking

	// check the head is not mining block
	BlockHeader* lastHeader = generator->getTopBlockHeader(); __STP(lastHeader);
	BlockHeader* targetLastBlock = nullptr;

	if(lastHeader->isScheduledBlock()){
		if(generator->continueMining()){ // if miner have to mine current block
			return;
		}

		// second to Top
		targetLastBlock = generator->changeMiningTarget();
	}
	else{
		targetLastBlock = dynamic_cast<BlockHeader*>(lastHeader->copyData());
	}
	__STP(targetLastBlock);

	// [consensus] wait for Pos Limit
	BlockchainController* ctrl = generator->getBlockchainController();
	if(ctrl != nullptr){
		uint64_t lastBlockHeight = targetLastBlock->getHeight();
		uint16_t zone = targetLastBlock->getZone();

		SystemTimestamp* limit = ctrl->getPosVoteLimit(zone, lastBlockHeight); __STP(limit);
		SystemTimestamp now;
		if(limit != nullptr && limit->compareTo(&now) > 0){
			// [Consensusu] Wait
			Os::usleep(200 * 1000);
			manager->resendMinerRequest();
			return;
		}
	}

	// gnerate new Block to mine
	const BlockHeaderId* lastBlockHeaderId = targetLastBlock->getId();
	uint64_t lastBlockHeight = targetLastBlock->getHeight();
	Block* block = generator->generateBlock(lastBlockHeight, lastBlockHeaderId, memTrx); __STP(block);

	// update mining scheduled block
	generator->setScheduledBlock(block);

	{
		AbstractPoWCalculator* calc = manager->getCalculator();

		const SystemTimestamp* sysTm = block->getTimestamp();

		BigInteger targetDifficulty = generator->calcTargetDiff(lastBlockHeight, lastBlockHeaderId, sysTm);
		const BlockMerkleRoot* merkleRoot = block->getBody()->getMerkleRoot();

		calc->calculate(lastBlockHeight, lastBlockHeaderId, merkleRoot, sysTm, &targetDifficulty);
	}

}

} /* namespace codablecash */
