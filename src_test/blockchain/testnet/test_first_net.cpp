/*
 * test_first_net.cpp
 *
 *  Created on: 2023/08/17
 *      Author: iizuka
 */
#include <osenv/funcs.h>
#include "bc/CodablecashSystemParam.h"

#include "test_utils/t_macros.h"

#include "bc/DebugDefaultLogger.h"

#include "../wallet_util/WalletDriver.h"
#include "../../test_utils/TestPortSelection.h"

#include "bc_network_instance/ArrayNetworkSheeder.h"
#include "bc_network_instance/CodablecashNetworkNode.h"
#include "bc_network_instance/CodablecashNetworkNodeConfig.h"
#include "bc_network_instance/GenesisBalanceConfig.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_block_generator/MiningConfig.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc_p2p_info/P2pNodeRecord.h"

#include "pubsub/CommandPublisherSetting.h"
#include "../utils/DebugCodablecashSystemParamSetup.h"
#include "bc_p2p/StackHandshakeReleaser.h"

#include "numeric/BigInteger.h"
using namespace codablecash;

TEST_GROUP(TestFirstNetGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestFirstNetGroup, case01){
	CommandPublisherSettingStack __set(5, 300);

	StackTestPortGetter portSel;
	int port01 = portSel.allocPort();
	int port02 = portSel.allocPort();
	int port03 = portSel.allocPort();

	File projectFolder = this->env->testCaseDir();

	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);
	File* dirNode02 = projectFolder.get(L"node02"); __STP(dirNode02);
	File* dirNode03 = projectFolder.get(L"node03"); __STP(dirNode03);

	DebugDefaultLogger logger;
	logger.addSection(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE);
	logger.addSection(ISystemLogger::DEBUG_POW_CALC_THREAD);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig02(param);

	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&param);

	// network config
	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);
	config01->setCanonicalName(L"node_01");
	CodablecashNetworkNodeConfig* config02 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config02);
	config02->setOriginalNwtworkKey();
	config02->setCanonicalName(L"node_02");
	CodablecashNetworkNodeConfig* config03 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config03);
	config03->setOriginalNwtworkKey();
	config03->setCanonicalName(L"node_03");

	WalletDriver* walletDriver = new WalletDriver(0, dirWallet01, L"changeit"); __STP(walletDriver);
	walletDriver->init(1);
	{
		GenesisBalanceConfig gconfig;
		AddressDescriptor* desc = walletDriver->getAddressDesc(0, 0); __STP(desc);
		BalanceUnit amount(1000L * 10000L * 10000L);
		gconfig.addBalance(amount, desc);

		config01->setGenesisConfig(&gconfig);
		config01->setPort(port01);

		{
			MiningConfig minerConfig;
			AddressDescriptor* desc2 = walletDriver->getAddressDesc(0, 1); __STP(desc2);
			minerConfig.setAddressDescriptor(desc2);
			config01->setMinerConfig(&minerConfig);
		}

		NodeIdentifierSource* networkKey =  NodeIdentifierSource::create(); __STP(networkKey);
		config01->setNetworkKey(networkKey);
	}

	//
	NodeIdentifier nodeId01 = config01->getNodeId();
	NodeIdentifier nodeId02 = config02->getNodeId();
	NodeIdentifier nodeId03 = config03->getNodeId();


	ArrayNetworkSheeder seeder;

	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	{
		// First
		node01.setNodeName(L"node01");
		node01.generateNetwork(0); // zone 0

		// after init
		node01.startNetwork(&seeder, false);

		node01.startMiner();
		node01.startStakePool();

		{
			uint16_t zone = 0;
			const NodeIdentifierSource* source = config01->getNetworkKey();
			NodeIdentifier nodeId = source->toNodeIdentifier();
			const wchar_t* host = L"::1";
			int port = config01->getPort();

			//UnicodeString idstr = nodeId.getPublicKey()->toString(16);

			const UnicodeString* cname = config01->getCanonicalName();
			P2pNodeRecord* rec = P2pNodeRecord::createIpV6Record(zone, &nodeId, cname, host, port); __STP(rec);
			seeder.addRecord(rec);
		}
	}

	{
		config02->setPort(port02);
	}
	CodablecashNetworkNode node02(dirNode02, config02, &logger);
	{
		// second
		node02.initBlank(0); // zone 0

		//UnicodeString idstr = nodeId01.getPublicKey()->toString(16);

		// after init
		node02.setNodeName(L"node02");
		node02.startNetwork(&seeder, true);
		node02.syncNetwork();

		// test commections
		{
			// node01
			{
				BlockchainNodeHandshake* hs01 = node01.getNodeHandshakeByNodeId(&nodeId01);
				StackHandshakeReleaser __releaser(hs01);
				CHECK(hs01 == nullptr);
			}
			{
				BlockchainNodeHandshake* hs02 = node01.getNodeHandshakeByNodeId(&nodeId02);
				StackHandshakeReleaser __releaser(hs02);
				CHECK(hs02 != nullptr);
			}

			// node02
			{
				BlockchainNodeHandshake* hs01 = node02.getNodeHandshakeByNodeId(&nodeId01);
				StackHandshakeReleaser __releaser(hs01);
				CHECK(hs01 != nullptr);
			}
			{
				BlockchainNodeHandshake* hs02 = node02.getNodeHandshakeByNodeId(&nodeId02);
				StackHandshakeReleaser __releaser(hs02);
				CHECK(hs02 == nullptr);
			}
		}


		// update seeder
		{
			uint16_t zone = 0;
			const NodeIdentifierSource* source = config02->getNetworkKey();
			NodeIdentifier nodeId = source->toNodeIdentifier();
			const wchar_t* host = L"::1";
			int port = config02->getPort();

			const UnicodeString* cname = config02->getCanonicalName();
			P2pNodeRecord* rec = P2pNodeRecord::createIpV6Record(zone, &nodeId, cname, host, port); __STP(rec);
			seeder.addRecord(rec);
		}
	}

	{
		config03->setPort(port03);
	}
	CodablecashNetworkNode node03(dirNode03, config03, &logger);
	{
		// third
		node03.initBlank(0); // zone 0

		// after init
		node03.setNodeName(L"node03");
		node03.startNetwork(&seeder, true);

		node03.syncNetwork();

		// test connections
		{
			// node01
			{
				BlockchainNodeHandshake* hs01 = node01.getNodeHandshakeByNodeId(&nodeId01);
				StackHandshakeReleaser __releaser(hs01);
				CHECK(hs01 == nullptr);
			}
			{
				BlockchainNodeHandshake* hs02 = node01.getNodeHandshakeByNodeId(&nodeId02);
				StackHandshakeReleaser __releaser(hs02);
				CHECK(hs02 != nullptr);
			}
			{
				BlockchainNodeHandshake* hs03 = node01.getNodeHandshakeByNodeId(&nodeId03);
				StackHandshakeReleaser __releaser(hs03);
				CHECK(hs03 != nullptr);
			}
			// node02
			{
				BlockchainNodeHandshake* hs01 = node02.getNodeHandshakeByNodeId(&nodeId01);
				StackHandshakeReleaser __releaser(hs01);
				CHECK(hs01 != nullptr);
			}
			{
				BlockchainNodeHandshake* hs02 = node02.getNodeHandshakeByNodeId(&nodeId02);
				StackHandshakeReleaser __releaser(hs02);
				CHECK(hs02 == nullptr);
			}
			{
				BlockchainNodeHandshake* hs03 = node02.getNodeHandshakeByNodeId(&nodeId03);
				StackHandshakeReleaser __releaser(hs03);
				CHECK(hs03 != nullptr);
			}
			// node03
			{
				BlockchainNodeHandshake* hs01 = node03.getNodeHandshakeByNodeId(&nodeId01);
				StackHandshakeReleaser __releaser(hs01);
				CHECK(hs01 != nullptr);
			}
			{
				BlockchainNodeHandshake* hs02 = node03.getNodeHandshakeByNodeId(&nodeId02);
				StackHandshakeReleaser __releaser(hs02);
				CHECK(hs02 != nullptr);
			}
			{
				BlockchainNodeHandshake* hs03 = node03.getNodeHandshakeByNodeId(&nodeId03);
				StackHandshakeReleaser __releaser(hs03);
				CHECK(hs03 == nullptr);
			}
		}
	}


	int maxHeight = 0;
	while(maxHeight < 5){
		Os::usleep(100000);
		maxHeight = node03.getMaxHeight(0);
	}

	node01.suspendMining();

	node03.shutdown();
	node02.shutdown();
	node01.shutdown();
}
