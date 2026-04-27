/*
 * test_pow_manager.cpp
 *
 *  Created on: 2023/03/22
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "pow/PoWManager.h"
#include "pow/PoWNonce.h"

#include "numeric/BigInteger.h"

#include "bc_block/BlockHeader.h"

#include "bc/DebugDefaultLogger.h"

#include "base/StackRelease.h"

#include "bc_memorypool/MemoryPool.h"

#include "bc_block_generator/MiningConfig.h"

#include "TestBlockGenerator.h"

using namespace codablecash;

TEST_GROUP(TestPoWManagerGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

/***
 * if(lastHeader->isScheduledBlock()) --> false
 */
TEST(TestPoWManagerGroup, case01){
	MiningConfig mconfig;

	File projectFolder = this->env->testCaseDir();

	DebugDefaultLogger logger;
	PoWManager manager(&logger, &mconfig);

	File* memdir = projectFolder.get(L"mempool"); __STP(memdir);
	MemoryPool memPool(memdir);

	BlockHeader header;
	header.setHeight(2);
	header.buildHeaderId();
	const BlockHeaderId* id = header.getId();

	BigInteger targetDiff(10000000L);

	TestBlockGenerator generator;
	generator.setMemoryPool(&memPool);
	generator.setTargetDiff(targetDiff);

	manager.setBlockGenerator(&generator);

	{
		manager.start();
		manager.requestNewHeaderId();

		generator.waitUntilGetData();

		const PoWNonce* nonce = generator.getNonce();
		BigInteger diff = nonce->getDifficulty();

		CHECK(targetDiff.compareTo(diff) < 0);

		manager.shutdown();
	}
}

/***
 * in PoWStartCalculateCommandMessage::process(PoWManager *manager)
 * if(lastHeader->isScheduledBlock()) --> true
 * if(generator->continueMining()) --> false
 */
TEST(TestPoWManagerGroup, case02){
	File projectFolder = this->env->testCaseDir();

	MiningConfig mconfig;
	DebugDefaultLogger logger;
	PoWManager manager(&logger, &mconfig);

	File* memdir = projectFolder.get(L"mempool"); __STP(memdir);
	MemoryPool memPool(memdir);

	BlockHeader header;
	header.setHeight(2);
	header.buildHeaderId();
	const BlockHeaderId* id = header.getId();

	BigInteger targetDiff(10000000L);

	TestBlockGenerator generator;
	generator.setMemoryPool(&memPool);
	generator.setTargetDiff(targetDiff);
	generator.setContinueMining(false);
	generator.setMaxNonce();

	manager.setBlockGenerator(&generator);

	{
		manager.start();
		manager.requestNewHeaderId();

		generator.waitUntilGetData();

		const PoWNonce* nonce = generator.getNonce();
		BigInteger diff = nonce->getDifficulty();

		CHECK(targetDiff.compareTo(diff) < 0);

		manager.shutdown();
	}
}

/***
 * in PoWStartCalculateCommandMessage::process(PoWManager *manager)
 * if(lastHeader->isScheduledBlock()) --> true
 * if(generator->continueMining()) --> true
 */
TEST(TestPoWManagerGroup, case03){
	File projectFolder = this->env->testCaseDir();

	MiningConfig mconfig;
	DebugDefaultLogger logger;
	PoWManager manager(&logger, &mconfig);

	File* memdir = projectFolder.get(L"mempool"); __STP(memdir);
	MemoryPool memPool(memdir);

	BlockHeader header;
	header.setHeight(2);
	header.buildHeaderId();
	const BlockHeaderId* id = header.getId();

	BigInteger targetDiff(10000000L);

	TestBlockGenerator generator;
	generator.setMemoryPool(&memPool);
	generator.setTargetDiff(targetDiff);
	generator.setContinueMining(true);
	generator.setMaxNonce();

	manager.setBlockGenerator(&generator);

	{
		manager.start();
		manager.requestNewHeaderId();

		manager.shutdown();
	}
}

