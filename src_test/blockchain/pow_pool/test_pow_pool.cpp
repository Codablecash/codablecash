/*
 * test_pow_pool.cpp
 *
 *  Created on: Apr 16, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "bc/DebugDefaultLogger.h"

#include "pow/PoWManager.h"
#include "pow/PoWNonce.h"

#include "pow_pool_client/PoWWorkerClient.h"

#include "bc_block_generator/MiningConfig.h"

#include "bc_memorypool/MemoryPool.h"

#include "bc_block/BlockHeader.h"

#include "../pow/TestBlockGenerator.h"

#include "../../test_utils/TestPortSelection.h"
#include "ipconnect/IServerSocket.h"

#include "bc_p2p_info/P2pNodeRecord.h"

using namespace codablecash;

TEST_GROUP(TestPoWPoolGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestPoWPoolGroup, case01){
	File projectFolder = this->env->testCaseDir();

	StackTestPortGetter portSel;
	int port = portSel.allocPort();

	MiningConfig mconfig;
	{
		mconfig.setCalclatorType(MiningConfig::TYPE_POOL);

		mconfig.setProtocol(P2pNodeRecord::TCP_IP_V4);
		UnicodeString host(L"127.0.0.1");
		mconfig.setServerHost(&host);
		mconfig.setPort(port);
	}


	DebugDefaultLogger logger;
	logger.addSection(ISystemLogger::DEBUG_POOL_TRANSFER_RESPONSE);
	PoWManager manager(&logger, &mconfig);

	File* memdir = projectFolder.get(L"mempool"); __STP(memdir);
	MemoryPool memPool(memdir);

	BlockHeader header;
	header.setHeight(2);
	header.buildHeaderId();
	const BlockHeaderId* id = header.getId();

	BigInteger targetDiff(100L);

	TestBlockGenerator generator;
	generator.setMemoryPool(&memPool);
	generator.setTargetDiff(targetDiff);

	manager.setBlockGenerator(&generator);

	{
		manager.start();

		// start client
		PoWWorkerClient client(&logger, 8, 20000);
		UnicodeString host(L"127.0.0.1");
		client.start(P2pNodeRecord::TCP_IP_V4, &host, port);

		manager.requestNewHeaderId();

		generator.waitUntilGetData();

		const PoWNonce* nonce = generator.getNonce();
		BigInteger diff = nonce->getDifficulty();

		CHECK(targetDiff.compareTo(diff) < 0);

		client.shutdownMainThread();
		client.shutdownProcessor();
		client.shutdown();

		manager.shutdown();
	}
}

