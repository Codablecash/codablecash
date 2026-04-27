/*
 * test_vote_and_finalize.cpp
 *
 *  Created on: 2023/05/04
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"

#include "../utils/InstanceDriver.h"
#include "bc_block_generator/MiningConfig.h"

#include "bc_base/BalanceUnit.h"

#include "bc/DebugDefaultLogger.h"

#include "bc_trx_balance/BalanceTransferTransaction.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_block/Block.h"

#include "bc/CodablecashNodeInstance.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_block_validator/BlockValidator.h"

#include "bc_memorypool/MemoryPool.h"

#include "../utils/ValidationInstance.h"
#include "bc/ISystemLogger.h"

#include "../utils/DebugCodablecashSystemParamSetup.h"
#include "dummy/BlankBlockGeneratorAllocator.h"

using namespace codablecash;

TEST_GROUP(TestVoteAndFinalizeTicketGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestVoteAndFinalizeTicketGroup, case01){
	File projectFolder = this->env->testCaseDir();
	InstanceDriver driver(&projectFolder);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig01(param);
	param.setPowBlockTimeMills(10);

	driver.initWallet(1);
	driver.initInstance(&param);

	MiningConfig mconfig;
	driver.startMiner(&mconfig);
	driver.startFinalize();

	driver.suspendMiner();

	driver.initGenesisBalance(0, 10000L * 1000L * 1000L);
	driver.registerVotingNode();


	int maxLoop = 30;
	for(int i = 0; i != maxLoop; ++i) {
		BalanceUnit stakeAmount(1000L);
		driver.registerTicket(stakeAmount);
	}

	BalanceTransferTransaction* badBalance = nullptr;
	{
		BalanceUnit feeRate(1L);
		BalanceUnit amount(100L);
		badBalance = driver.createBalanceTransferTransaction(amount, feeRate);
	}
	__STP(badBalance);

	{
		BalanceUnit feeRate(2L);
		BalanceUnit amount(100L);
		driver.sendBalance(amount, feeRate);
	}
	{
		BalanceUnit feeRate(3L);
		BalanceUnit amount(200L);
		driver.sendBalance(amount, feeRate);
	}

	{
		BlockchainController* ctrl = driver.getBlockchainController();
		int count = ctrl->getMempoolTrxCount();
		CHECK(count == 33)
	}


	driver.resumeMiner();

	driver.waitUntilHeight(3);

	{
		driver.transferAbstractBalanceTransaction(badBalance);

		BalanceUnit amount(100L);
		badBalance->setFeeAmount(&amount);
		bool valid = driver.validateTransaction(badBalance);
		CHECK(valid == false);
	}

	uint64_t wheight = 5;
	driver.waitUntilFinalizedHeight(wheight);
	driver.suspendMiner();

	// Test validator
	{
		DebugDefaultLogger* logger = driver.getLogger();
		ValidationInstance validator(&projectFolder);
		validator.initInstance(&param, logger);

		BlockchainController* ctrl = driver.getBlockchainController();

		uint64_t maxH = wheight + 1;
		for(uint64_t h = 1; h != maxH; h++){
			ArrayList<Block>* list = ctrl->getBlocksHeightAt(0, h); __STP(list);
			list->setDeleteOnExit();

			int maxLoop = list->size();
			for(int i = 0; i != maxLoop; ++i){
				const Block* block = list->get(i);

				validator.validateBlock(block);
			}
		}
	}
}

/**
 * use BlankBlockGenerator
 * Also tested BlockchainController#checkAcceptSecondRealBlockOnMining()
 */
TEST(TestVoteAndFinalizeTicketGroup, case02){
	File projectFolder = this->env->testCaseDir();
	InstanceDriver driver(&projectFolder);

	driver.getLogger()->setSection(ISystemLogger::DEBUG_CHAIN_HEAD_DETECT);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig01(param);
	param.setPowBlockTimeMills(10);

	driver.initWallet(1);

	BlankBlockGeneratorAllocator alloc;
	driver.initInstance(&param, &alloc);

	MiningConfig mconfig;
	driver.startMiner(&mconfig);
	driver.startFinalize();

	driver.suspendMiner();

	driver.initGenesisBalance(0, 10000L * 1000L * 1000L);
	driver.registerVotingNode();


	int maxLoop = 30;
	for(int i = 0; i != maxLoop; ++i) {
		BalanceUnit stakeAmount(1000L);
		driver.registerTicket(stakeAmount);
	}

	{
		BalanceUnit feeRate(1L);
		BalanceUnit amount(100L);
		driver.sendBalance(amount, feeRate);
	}

	driver.resumeMiner();


	//driver.waitUntilHeight(3);
	uint64_t wheight = 4;
	driver.waitUntilFinalizedHeight(wheight);
	driver.suspendMiner();

}

