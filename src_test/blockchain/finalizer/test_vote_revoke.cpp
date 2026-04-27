/*
 * test_vote_revoke.cpp
 *
 *  Created on: 2023/05/24
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"

#include "../utils/InstanceDriver.h"
#include "bc_block_generator/MiningConfig.h"

#include "bc_base/BalanceUnit.h"

#include "bc/DebugDefaultLogger.h"

#include "bc/CodablecashNodeInstance.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_block/BlockHeaderId.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "../utils/ValidationInstance.h"
#include "bc_block/Block.h"

#include "../utils/DebugCodablecashSystemParamSetup.h"
#include "dummy/FinalizeMissAndBlankblockTestAllocator.h"

using namespace codablecash;

TEST_GROUP(TestVoteRevokeTicketGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};


/**
 *　use FinalizeMissTestPool#requestCheckVoting() for miss vote
 *　use FinalizeMissDetectTicketCommandMessage#putTransaction() make wrong vote transaction
 *
 */
TEST(TestVoteRevokeTicketGroup, case01){
	File projectFolder = this->env->testCaseDir();
	InstanceDriver driver(&projectFolder);

	driver.getLogger()->setSection(ISystemLogger::DEBUG_CHAIN_HEAD_DETECT);

	CodablecashSystemParam config;
	DebugCodablecashSystemParamSetup::setupConfig01(config);
	config.setPowBlockTimeMills(10);

	driver.initWallet(1);

	FinalizeMissAndBlankblockTestAllocator alloc;
	driver.initInstance(&config, &alloc);

	MiningConfig mconfig;
	driver.startMiner(&mconfig);
	driver.startFinalize();

	driver.suspendMiner();

	driver.initGenesisBalance(0, 10000L * 1000L * 1000L);
	driver.registerVotingNode();

	int maxLoop = 40;
	for(int i = 0; i != maxLoop; ++i) {
		BalanceUnit stakeAmount(1000L);
		driver.registerTicket(stakeAmount);
	}

	{
		BalanceUnit feeRate(2L);
		BalanceUnit amount(100L);
		driver.sendBalance(amount, feeRate);
	}
	{
		BalanceUnit feeRate(1L);
		BalanceUnit amount(200L);
		driver.sendBalance(amount, feeRate);
	}

	driver.resumeMiner();


	uint64_t wheight = 5;
	driver.waitUntilFinalizedHeight(wheight);
	driver.suspendMiner();


	// misc
	{
		CodablecashNodeInstance* nodeInst = driver.getInstance();
		BlockchainController* ctrl = nodeInst->getController();
		CodablecashBlockchain* chain = ctrl->getCodablecashBlockchain();
		BlockHeaderStoreManager* mgr = chain->getHeaderManager(0);

		char data[32]{};
		BlockHeaderId headerId(data, 32);

		BlockHeader* h = mgr->getHeader(&headerId, 1000L);
		CHECK(h == nullptr);
	}
}


