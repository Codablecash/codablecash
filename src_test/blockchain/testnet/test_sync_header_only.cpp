/*
 * test_sync_header_only.cpp
 *
 *  Created on: 2023/12/12
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"

#include "../utils_testnet/MultizoneTestnet.h"
#include "../utils_testnet/TestnetInstanceWrapper.h"
#include "../utils_testnet/ArrayDebugSeeder.h"

#include "../../test_utils/TestPortSelection.h"
#include "../utils/DebugCodablecashSystemParamSetup.h"
#include "../wallet_util/WalletDriver.h"

#include "pubsub/CommandPublisherSetting.h"

#include "bc/DebugDefaultLogger.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_network_instance/CodablecashNetworkNodeConfig.h"
#include "bc_network_instance/GenesisBalanceConfig.h"
#include "bc_network_instance/FinalizerConfig.h"
#include "bc_network_instance/CodablecashNetworkNode.h"

#include "bc_network/NodeIdentifierSource.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_block_generator/MiningConfig.h"


#include "bc_blockstore/InvalidZoneException.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc/CodablecashNodeInstance.h"

#include "osenv/funcs.h"

#include "bc_blockstore_header/BlockHeaderNotFoundException.h"
using namespace codablecash;

TEST_GROUP(TestSyncHeaderOnlyGroup) {
	TEST_SETUP(){
		env->setup();
		this->val = 0;
	}
	TEST_TEARDOWN(){
		env->teardown();
	}

private:
	int val;
};

TEST(TestSyncHeaderOnlyGroup, case01){
	CommandPublisherSettingStack __set(5, 300);

	StackTestPortGetter portSel;
	File projectFolder = this->env->testCaseDir();
	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);

	DebugDefaultLogger logger;
	logger.addSection(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE);
	// logger.setSection(ISystemLogger::DEBUG_POW_CALC_THREAD);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig02(param);

	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&param);

	// host;
	UnicodeString localhost(L"localhost");
	nwconfig.setHost(&localhost);

	MultizoneTestnet testnet(&portSel, &projectFolder, &logger, &param, &nwconfig);

	// wallet
	WalletDriver* walletDriver = new WalletDriver(0, dirWallet01, L"changeit"); __STP(walletDriver);
	walletDriver->init(1);

	// genesis address
	AddressDescriptor* desc = walletDriver->getAddressDesc(0, 0); __STP(desc);
	GenesisBalanceConfig gconfig;
	BalanceUnit amount(1000L * 10000L * 10000L);
	gconfig.addBalance(amount, desc);


	{
		TestnetInstanceWrapper* inst = testnet.createInstance(0, L"first");
		// genesis
		inst->setGenesisBlockConfig(&gconfig);

		// mining
		MiningConfig minerConfig;
		AddressDescriptor* desc2 = walletDriver->getAddressDesc(0, 1); __STP(desc2);
		minerConfig.setAddressDescriptor(desc2);
		inst->setMinerConfig(&minerConfig);

		// finalizer
		FinalizerConfig fconfig;
		NodeIdentifierSource* voteSource = NodeIdentifierSource::create(); __STP(voteSource);
		fconfig.setVoterSource(voteSource);
		inst->setFinalizerConfig(&fconfig);
	}

	// seeder
	ArrayDebugSeeder seeder;

	testnet.startGenesis(0, &seeder);
	testnet.waitForBlockHeight(0, 0, 4);

	// header
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);
	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);
	int port01 = portSel.allocPort();
	config01->setPort(port01);

	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	{
		// second
		node01.initBlank(1); // zone 0

		// after init
		node01.setNodeName(L"node02");
		node01.startNetwork(&seeder, true);

		node01.syncNetwork();

		CodablecashNodeInstance* inst = node01.getInstance();

		BlockchainController* ctrl = inst->getController();
		uint64_t h = ctrl->getHeadHeight(0);

		while(h <= 6){
			Os::usleep(100*1000);
			h = ctrl->getHeadHeight(0);
		}
	}

	node01.broadCastShutdownCommand();
}

TEST(TestSyncHeaderOnlyGroup, InvalidZoneException01){
	testException<InvalidZoneException>();
	testException<BlockHeaderNotFoundException>();
}



