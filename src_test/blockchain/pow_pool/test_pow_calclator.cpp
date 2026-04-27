/*
 * test_pow_calclator.cpp
 *
 *  Created on: Apr 14, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "bc/DebugDefaultLogger.h"

#include "pow/PoWManager.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/BlockMerkleRoot.h"

#include "bc_block_generator/MiningConfig.h"

#include "pow_pool_client/PoWWorkerClient.h"
#include "pow_pool_client/PoWNonceProcessor.h"

#include "pow_pool/PoWRequestData.h"

#include "base_timestamp/SystemTimestamp.h"

#include "numeric/BigInteger.h"

using namespace codablecash;

TEST_GROUP(TestPoWCalcGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestPoWCalcGroup, case01) {
	DebugDefaultLogger logger;

	PoWWorkerClient* client = new PoWWorkerClient(&logger, 8, 40000); __STP(client);

	PoWNonceProcessor* processor = new PoWNonceProcessor(client, 4, &logger); __STP(processor);
	PoWRequestData data;

	uint64_t lastBlockHeight = 10;
	BlockHeaderId* lastHeaderId = BlockHeaderId::makeRandomHeaderId(); __STP(lastHeaderId);
	BlockMerkleRoot *merkleRoot = BlockMerkleRoot::createZeroRoot(); __STP(merkleRoot);
	SystemTimestamp tm;
	BigInteger target(10000L);
	data.init(lastBlockHeight, lastHeaderId, merkleRoot, &tm, &target);

	BigInteger solt(100);
	processor->start(&data, &solt, 40000);

	processor->shutdown();
}

