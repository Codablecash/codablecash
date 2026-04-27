/*
 * test_vote_revoke_validation.cpp
 *
 *  Created on: 2023/12/03
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

#include "bc_block_body/BlockBody.h"

#include "bc_block_body/BlockRewordBase.h"

#include "bc_block_validator/BlockValidationException.h"

#include "../utils/DebugCodablecashSystemParamSetup.h"
#include "dummy/FinalizeMissTestAllocator.h"
#include "dummy/FinalizeMissTestAllocator02.h"

using namespace codablecash;

/***
 * This test do not generate dummy blank block with PoW
 */
TEST_GROUP(TestVoteRevokeTicketValidationGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};


/**
 *  BlankBlockWithCoinbaseGenerator#nonceCalculated(); pow
 *  FinalizeMissTestPool -> FinalizeMissDetectTicketCommandMessage (wrong vote)
 *
 */
TEST(TestVoteRevokeTicketValidationGroup, case01){
	File projectFolder = this->env->testCaseDir();
	InstanceDriver driver(&projectFolder);

	driver.getLogger()->setSection(ISystemLogger::DEBUG_CHAIN_HEAD_DETECT);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig01(param);
	param.setPowBlockTimeMills(10);

	driver.initWallet(1);

	FinalizeMissTestAllocator alloc;
	driver.initInstance(&param, &alloc);

	MiningConfig mconfig;
	driver.startMiner(&mconfig);
	driver.startFinalize();

	driver.suspendMiner();

	driver.initGenesisBalance(0, 10000L * 1000L * 1000L);
	driver.registerVotingNode();

	{
		int maxLoop = 40;
		for(int i = 0; i != maxLoop; ++i) {
			BalanceUnit stakeAmount(1000L);
			driver.registerTicket(stakeAmount);
		}
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

	// Test validator
	/**
	 * Test validater's instance does not have memory pool.
	 * Therefore voted  block has -5 mev health score
	 */
	{
		DebugDefaultLogger* logger = driver.getLogger();
		ValidationInstance validator(&projectFolder);
		validator.initInstance(&param, logger);

		BlockchainController* ctrl = driver.getBlockchainController();

		uint64_t maxH = wheight + param.getVoteBeforeNBlocks(wheight) + param.getVoteBlockIncludeAfterNBlocks(wheight) +1;
		for(uint64_t h = 1; h != maxH; h++){
			ArrayList<Block>* list = ctrl->getBlocksHeightAt(0, h); __STP(list);
			list->setDeleteOnExit();

			bool success = false;
			int maxLoop = list->size();
			for(int i = 0; i != maxLoop; ++i){
				const Block* block = list->get(i);
				BlockBody* body = block->getBody();
				const BlockRewordBase* base = body->getBlockRewordBase();
				const CoinbaseTransaction* coinbase = base->getCoinbaseTransaction();

				try{
					validator.validateBlock(block);
				}
				catch(BlockValidationException* e){
					delete e;
					continue;
				}

				success = true;
			}

			CHECK(success);
		}
	}
}


/**
 *	BlankBlockWithCoinbaseGenerator02
 *	-> BlankBlockWithCoinbaseGenerator02::nonceCalculated()
 *　FinalizeMissTestPool -> FinalizeMissDetectTicketCommandMessage (wrong vote)
 *
 */
TEST(TestVoteRevokeTicketValidationGroup, case02){
	File projectFolder = this->env->testCaseDir();
	InstanceDriver driver(&projectFolder);

	driver.getLogger()->setSection(ISystemLogger::DEBUG_CHAIN_HEAD_DETECT);

	CodablecashSystemParam config;
	DebugCodablecashSystemParamSetup::setupConfig01(config);
	config.setPowBlockTimeMills(10);

	driver.initWallet(1);

	FinalizeMissTestAllocator02 alloc;
	driver.initInstance(&config, &alloc);

	MiningConfig mconfig;
	driver.startMiner(&mconfig);
	driver.startFinalize();

	driver.suspendMiner();

	driver.initGenesisBalance(0, 10000L * 1000L * 1000L);
	driver.registerVotingNode();

	{
		int maxLoop = 40;
		for(int i = 0; i != maxLoop; ++i) {
			BalanceUnit stakeAmount(1000L);
			driver.registerTicket(stakeAmount);
		}
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

	// Test validator

	{
		DebugDefaultLogger* logger = driver.getLogger();
		ValidationInstance validator(&projectFolder);
		validator.initInstance(&config, logger);

		BlockchainController* ctrl = driver.getBlockchainController();

		uint64_t maxH = wheight + config.getVoteBeforeNBlocks(wheight) + config.getVoteBlockIncludeAfterNBlocks(wheight);
		for(uint64_t h = 1; h != maxH; h++){
			ArrayList<Block>* list = ctrl->getBlocksHeightAt(0, h); __STP(list);
			list->setDeleteOnExit();

			bool success = false;
			int maxLoop = list->size();
			for(int i = 0; i != maxLoop; ++i){
				const Block* block = list->get(i);
				BlockBody* body = block->getBody();
				const BlockRewordBase* base = body->getBlockRewordBase();
				const CoinbaseTransaction* coinbase = base->getCoinbaseTransaction();

				try{
					validator.validateBlock(block);
				}
				catch(BlockValidationException* e){
					delete e;
					continue;
				}

				success = true;
			}

			CHECK(success);
		}
	}
}

