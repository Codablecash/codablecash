/*
 * test_p2p_network_cmd.cpp
 *
 *  Created on: 2023/09/09
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../../test_utils/TestPortSelection.h"
#include "../../blockchain/wallet_util/WalletDriver.h"

#include "bc/DebugDefaultLogger.h"
#include "bc/CodablecashNodeInstance.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc_network_instance/CodablecashNetworkNodeConfig.h"
#include "bc_network_instance/ArrayNetworkSheeder.h"
#include "bc_network_instance/CodablecashNetworkNode.h"
#include "bc_network_instance/GenesisBalanceConfig.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_status_cache/BlockchainController.h"
#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "base/ArrayList.h"
#include "base/Exception.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_block/Block.h"

#include "bc_p2p_cmd/P2pHandshakeAuthenticationException.h"

#include "bc_p2p/BlochchainP2pManager.h"
#include "bc_p2p/BlockchainNodeHandshake.h"
#include "bc_p2p/StackHandshakeReleaser.h"

#include "bc/ExceptionThrower.h"
#include "../../blockchain/utils/DebugCodablecashSystemParamSetup.h"


using namespace codablecash;

TEST_GROUP(TestP2pNetworkGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestP2pNetworkGroup, case01){
	File projectFolder = this->env->testCaseDir();

	StackTestPortGetter portSel;
	int port01 = portSel.allocPort();
	int port02 = portSel.allocPort();
	int port03 = portSel.allocPort();

	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);
	File* dirNode02 = projectFolder.get(L"node02"); __STP(dirNode02);

	DebugDefaultLogger logger;

	CodablecashSystemParam config;
	DebugCodablecashSystemParamSetup::setupConfig01(config);
	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&config);

	WalletDriver* walletDriver = new WalletDriver(0, dirWallet01, L"changeit"); __STP(walletDriver);
	walletDriver->init(1);
	{
		GenesisBalanceConfig gconfig;
		AddressDescriptor* desc = walletDriver->getAddressDesc(0, 0); __STP(desc);
		BalanceUnit amount(1000L * 10000L * 10000L);
		gconfig.addBalance(amount, desc);

		nwconfig.setGenesisConfig(&gconfig);
	}

	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);
	CodablecashNetworkNodeConfig* config02 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config02);

	ArrayNetworkSheeder seeder;

	config01->setPort(port01);
	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	{
		// First
		node01.generateNetwork(0, 1); // zone 0
		// after init
		node01.startNetwork(&seeder, false);
	}

	config02->setPort(port02);
	CodablecashNetworkNode node02(dirNode02, &nwconfig, &logger);
	{
		// First
		node02.initBlank(0, 1); // zone 0

		// after init
		node02.startNetwork(&seeder, false);
		{
			CodablecashNodeInstance* inst = node01.getInstance();
			BlockchainController* ctrl = inst->getController();
			CodablecashBlockchain* chain = ctrl->getCodablecashBlockchain();

			ArrayList<Block>* list = chain->getBlocksHeightAt(0, 1); __STP(list);
			list->setDeleteOnExit();

			{
				BlockchainController* ctrl2 = node02.getInstance()->getController();
				const Block* block = list->get(0);
				ctrl2->addBlock(block);
			}
		}
	}

	// connect
	{
		CodablecashNodeInstance* inst = node02.getInstance();

		UnicodeString strLocal(L"::1");
		NodeIdentifier nodeId = config01->getNodeId();
		inst->connectIpV6Node(0, &strLocal, port01, &nodeId, nullptr);
	}

	node02.shutdown();
	node01.shutdown();
}

TEST(TestP2pNetworkGroup, case02_err){
	File projectFolder = this->env->testCaseDir();

	StackTestPortGetter portSel;
	int port01 = portSel.allocPort();
	int port02 = portSel.allocPort();
	int port03 = portSel.allocPort();

	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);
	File* dirNode02 = projectFolder.get(L"node02"); __STP(dirNode02);

	DebugDefaultLogger logger;

	CodablecashSystemParam config;
	DebugCodablecashSystemParamSetup::setupConfig01(config);
	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&config);

	WalletDriver* walletDriver = new WalletDriver(0, dirWallet01, L"changeit"); __STP(walletDriver);
	walletDriver->init(1);
	{
		GenesisBalanceConfig gconfig;
		AddressDescriptor* desc = walletDriver->getAddressDesc(0, 0); __STP(desc);
		BalanceUnit amount(1000L * 10000L * 10000L);
		gconfig.addBalance(amount, desc);

		nwconfig.setGenesisConfig(&gconfig);
	}

	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);
	CodablecashNetworkNodeConfig* config02 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config02);

	ArrayNetworkSheeder seeder;

	config01->setPort(port01);
	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	{
		// First
		node01.generateNetwork(0, 1); // zone 0
		// after init
		node01.startNetwork(&seeder, false);
	}

	config02->setPort(port02);
	CodablecashNetworkNode node02(dirNode02, &nwconfig, &logger);
	{
		// First
		node02.initBlank(0, 1); // zone 0

		// after init
		node02.startNetwork(&seeder, false);
		{
			CodablecashNodeInstance* inst = node01.getInstance();
			BlockchainController* ctrl = inst->getController();
			CodablecashBlockchain* chain = ctrl->getCodablecashBlockchain();

			ArrayList<Block>* list = chain->getBlocksHeightAt(0, 1); __STP(list);
			list->setDeleteOnExit();

			{
				BlockchainController* ctrl2 = node02.getInstance()->getController();
				const Block* block = list->get(0);
				ctrl2->addBlock(block);
			}
		}
	}

	// connect
	{
		CodablecashNodeInstance* inst = node02.getInstance();

		UnicodeString strLocal(L"::1");
		NodeIdentifier nodeId = config01->getNodeId();
		inst->connectIpV6Node(0, &strLocal, port01, &nodeId, nullptr);

		P2pHandshakeAuthenticationException* ex = nullptr;
		try{
			BlochchainP2pManager* p2pmgr = inst->getBlochchainP2pManager();
			NodeIdentifierSource* src = node01.getInstance()->getNetworkKey();
			NodeIdentifier n = src->toNodeIdentifier();

			BlockchainNodeHandshake* nodeHandshake = p2pmgr->getNodeHandshakeByNodeId(&n);
			StackHandshakeReleaser __releaser(nodeHandshake);

			P2pHandshake* handshake = nodeHandshake->getHandshake();

			inst->loginNode(0, handshake, nullptr);
		}catch(P2pHandshakeAuthenticationException* e){
			ex = e;
		}
		CHECK(ex != nullptr)
		delete ex;
	}

	node02.shutdown();
	node01.shutdown();
}

TEST(TestP2pNetworkGroup, case03_err){
	File projectFolder = this->env->testCaseDir();

	StackTestPortGetter portSel;
	int port01 = portSel.allocPort();
	int port02 = portSel.allocPort();
	int port03 = portSel.allocPort();

	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);
	File* dirNode02 = projectFolder.get(L"node02"); __STP(dirNode02);

	DebugDefaultLogger logger;

	CodablecashSystemParam config;
	DebugCodablecashSystemParamSetup::setupConfig01(config);
	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&config);

	WalletDriver* walletDriver = new WalletDriver(0, dirWallet01, L"changeit"); __STP(walletDriver);
	walletDriver->init(1);
	{
		GenesisBalanceConfig gconfig;
		AddressDescriptor* desc = walletDriver->getAddressDesc(0, 0); __STP(desc);
		BalanceUnit amount(1000L * 10000L * 10000L);
		gconfig.addBalance(amount, desc);

		nwconfig.setGenesisConfig(&gconfig);
	}

	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);
	CodablecashNetworkNodeConfig* config02 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config02);

	ArrayNetworkSheeder seeder;

	config01->setPort(port01);
	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	{
		// First
		node01.generateNetwork(0, 1); // zone 0
		// after init
		node01.startNetwork(&seeder, false);

	}

	config02->setPort(port02);
	CodablecashNetworkNode node02(dirNode02, &nwconfig, &logger);
	{
		// First
		node02.initBlank(0, 1); // zone 0

		// after init
		node02.startNetwork(&seeder, false);
		{
			CodablecashNodeInstance* inst = node01.getInstance();
			BlockchainController* ctrl = inst->getController();
			CodablecashBlockchain* chain = ctrl->getCodablecashBlockchain();

			ArrayList<Block>* list = chain->getBlocksHeightAt(0, 1); __STP(list);
			list->setDeleteOnExit();

			{
				BlockchainController* ctrl2 = node02.getInstance()->getController();
				const Block* block = list->get(0);
				ctrl2->addBlock(block);
			}
		}
	}

	// connect
	{
		ErrorPointManager* errmgr = ErrorPointManager::getInstance();
		errmgr->activatePoint(L"CodablecashNodeInstance::connectIpV6Node", L"BlochchainP2pManager::registerHandshake", 1);

		CodablecashNodeInstance* inst = node02.getInstance();

		UnicodeString strLocal(L"::1");

		Exception* ex = nullptr;
		try{
			NodeIdentifier nodeId = config01->getNodeId();
			inst->connectIpV6Node(0, &strLocal, port01, &nodeId, nullptr);
		}
		catch(Exception* e){
			ex = e;
		}
		CHECK(ex != nullptr);
		delete ex;
	}

	node02.shutdown();
	node01.shutdown();
}


TEST(TestP2pNetworkGroup, misc01){
	P2pHandshakeAuthenticationException* ex = nullptr;
	try{
		ExceptionThrower<P2pHandshakeAuthenticationException>::throwExceptionIfCondition(true, L"", __FILE__, __LINE__);
	}
	catch(P2pHandshakeAuthenticationException* e){
		ex = e;
	}
	CHECK(ex != nullptr);
	delete ex;
}
