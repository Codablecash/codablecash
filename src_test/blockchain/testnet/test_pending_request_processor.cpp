/*
 * test_pending_requestprocessor.cpp
 *
 *  Created on: 2023/11/14
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../../test_utils/TestPortSelection.h"
#include "../wallet_util/WalletDriver.h"
#include "../../p2p/p2pserver/dummy/DummyClientListner.h"
#include "../utils/ClientConnectUtils.h"

#include "bc/DebugDefaultLogger.h"
#include "bc/CodablecashNodeInstance.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_network_instance/CodablecashNetworkNodeConfig.h"
#include "bc_network_instance/ArrayNetworkSheeder.h"
#include "bc_network_instance/GenesisBalanceConfig.h"
#include "bc_network_instance/CodablecashNetworkNode.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc_block/Block.h"

#include "bc_block_body/BlockBody.h"

#include "bc_block_generator/MiningConfig.h"
#include "bc_block_validator/BlockValidationException.h"

#include "bc_p2p_client/P2pClient.h"

#include "bc_status_cache/BlockchainController.h"

#include "pubsub_cmd/ErrorPubsubResponse.h"

#include "bc_p2p_info/P2pNodeRecord.h"

#include "command_queue_cmd/NodeQueueCommandException.h"

#include "bc_p2p_processor/P2pRequestQueueProcessor.h"
#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "pubsub/PubSubId.h"

#include "bc_p2p_cmd_node/NopNodeCommand.h"

#include "command_queue_cmd/NopNodeQueueCommand.h"

#include "pubsub/CommandPublisherSetting.h"
#include "../utils/DebugCodablecashSystemParamSetup.h"

using namespace codablecash;

TEST_GROUP(TestPendingRequestProcessorGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

/**
 * This case success, but has possibility to cause following error.
 * Blockchain Transaction error. Invalid Transaction was sent. at Exception occurs at /home/iizuka/DATA/git/workspace/codablecash/src_blockchain_p2p/bc_p2p_cmd_node/SendTransactionNodeCommand.cpp line 117
 * */
TEST(TestPendingRequestProcessorGroup, case01){
	CommandPublisherSettingStack __set(5, 300);

	StackTestPortGetter portSel;
	int port01 = portSel.allocPort();
	int port02 = portSel.allocPort();

	File projectFolder = this->env->testCaseDir();
	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);
	File* dirNode02 = projectFolder.get(L"node02"); __STP(dirNode02);

	DebugDefaultLogger logger;
	logger.addSection(DebugDefaultLogger::DEBUG_NODE_TRANSFER_RESPONSE);
	logger.addSection(DebugDefaultLogger::DEBUG_TMP_INFO);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig02(param);

	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&param);

	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);
	CodablecashNetworkNodeConfig* config02 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config02);

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

	ArrayNetworkSheeder seeder;

	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	{
		// First
		node01.generateNetwork(0); // zone 0

		// after init
		node01.startNetwork(&seeder, false);
		node01.setNodeName(L"01");

		node01.startMiner();
		node01.startStakePool();

		// setup seeder
		{
			uint16_t zone = 0;
			const NodeIdentifierSource* source = config01->getNetworkKey();
			NodeIdentifier nodeId = source->toNodeIdentifier();
			const wchar_t* host = L"::1";
			int port = config01->getPort();

			P2pNodeRecord* rec = P2pNodeRecord::createIpV6Record(zone, &nodeId, nullptr, host, port); __STP(rec);
			seeder.addRecord(rec);
		}
	}
	//////////////
	// sync wallet
	{
		BlockchainController* ctrl = node01.getInstance()->getController();

		ArrayList<Block>* blocks = ctrl->getBlocksHeightAt(0, 1); __STP(blocks);
		blocks->setDeleteOnExit();

		int maxBlocks = blocks->size();
		for(int i = 0; i != maxBlocks; ++i){
			Block* block = blocks->get(i);
			BlockBody* body = block->getBody();

			const ArrayList<AbstractBalanceTransaction>* list = body->getBalanceTransactions();
			int maxLoop = list->size();
			for(int j = 0; j != maxLoop; ++j){
				AbstractBalanceTransaction* trx = list->get(j);
				walletDriver->importTransaction(trx);
			}
		}
	}


	///////////////
	{
		config02->setPort(port02);
	}

	{
		CodablecashNetworkNode node02(dirNode02, config02, &logger);

		// second
		node02.initBlank(0); // zone 0

		// after init
		node02.startNetwork(&seeder, true);
		node02.setNodeName(L"02");

		// send to node01
		DummyClientListner listner;
		{
			UnicodeString strLocal(L"::1");
			P2pClient* client = ClientConnectUtils::connect(&strLocal, port01, 0, &logger); __STP(client);
			client->addListner(&listner);

			AbstractCommandResponse* res = walletDriver->registerVotingNode(client); __STP(res);

			listner.waitUntilCount(1);
		}

		uint64_t lastHeight = node01.getMaxHeight(0);
		uint64_t height = node01.getMaxHeight(0);

		while(height - lastHeight < 4){
			Os::usleep(1000 * 100);
			height = node01.getMaxHeight(0);
		}
		node01.suspendMining();

		node02.syncNetwork();
	}
}

/**
 * handle unprocessed transactions
 */
TEST(TestPendingRequestProcessorGroup, case01_02){
	CommandPublisherSettingStack __set(5, 300);

	StackTestPortGetter portSel;
	int port01 = portSel.allocPort();
	int port02 = portSel.allocPort();

	File projectFolder = this->env->testCaseDir();
	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);
	File* dirNode02 = projectFolder.get(L"node02"); __STP(dirNode02);

	DebugDefaultLogger logger;
	logger.addSection(DebugDefaultLogger::DEBUG_NODE_TRANSFER_RESPONSE);
	logger.addSection(DebugDefaultLogger::DEBUG_TMP_INFO);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig02(param);

	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&param);

	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);
	CodablecashNetworkNodeConfig* config02 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config02);

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

	ArrayNetworkSheeder seeder;

	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	{
		// First
		node01.generateNetwork(0); // zone 0

		// after init
		node01.startNetwork(&seeder, false);
		node01.setNodeName(L"01");

		node01.startMiner();
		node01.startStakePool();

		// setup seeder
		{
			uint16_t zone = 0;
			const NodeIdentifierSource* source = config01->getNetworkKey();
			NodeIdentifier nodeId = source->toNodeIdentifier();
			const wchar_t* host = L"::1";
			int port = config01->getPort();

			P2pNodeRecord* rec = P2pNodeRecord::createIpV6Record(zone, &nodeId, nullptr, host, port); __STP(rec);
			seeder.addRecord(rec);
		}
	}
	//////////////
	// sync wallet
	{
		BlockchainController* ctrl = node01.getInstance()->getController();

		ArrayList<Block>* blocks = ctrl->getBlocksHeightAt(0, 1); __STP(blocks);
		blocks->setDeleteOnExit();

		int maxBlocks = blocks->size();
		for(int i = 0; i != maxBlocks; ++i){
			Block* block = blocks->get(i);
			BlockBody* body = block->getBody();

			const ArrayList<AbstractBalanceTransaction>* list = body->getBalanceTransactions();
			int maxLoop = list->size();
			for(int j = 0; j != maxLoop; ++j){
				AbstractBalanceTransaction* trx = list->get(j);
				walletDriver->importTransaction(trx);
			}
		}
	}


	///////////////
	{
		config02->setPort(port02);
	}

	{
		CodablecashNetworkNode node02(dirNode02, config02, &logger);

		// second
		node02.initBlank(0); // zone 0

		// after init
		node02.startNetwork(&seeder, true);
		node02.setNodeName(L"02");

		// send to node01
		DummyClientListner listner;
		{
			UnicodeString strLocal(L"::1");
			P2pClient* client = ClientConnectUtils::connect(&strLocal, port01, 0, &logger); __STP(client);
			client->addListner(&listner);

			AbstractCommandResponse* res = walletDriver->registerVotingNode(client); __STP(res);

			listner.waitUntilCount(1);
		}

		node01.suspendMining();
		node02.syncNetwork();
	}
}

TEST(TestPendingRequestProcessorGroup, case02_err){
	StackTestPortGetter portSel;
	int port01 = portSel.allocPort();
	int port02 = portSel.allocPort();

	File projectFolder = this->env->testCaseDir();
	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);

	DebugDefaultLogger logger;

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig02(param);

	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&param);

	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);

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

	ArrayNetworkSheeder seeder;

	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	{
		// First
		node01.generateNetwork(0); // zone 0

		// after init
		node01.startNetwork(&seeder, true);
		node01.setNodeName(L"01");
	}

	{
		BlockchainController* ctrl = node01.getInstance()->getController();

		ArrayList<Block>* blocks = ctrl->getBlocksHeightAt(0, 1); __STP(blocks);
		blocks->setDeleteOnExit();

		int maxBlocks = blocks->size();
		for(int i = 0; i != maxBlocks; ++i){
			Block* block = blocks->get(i);
			BlockBody* body = block->getBody();

			const ArrayList<AbstractBalanceTransaction>* list = body->getBalanceTransactions();
			int maxLoop = list->size();
			for(int j = 0; j != maxLoop; ++j){
				AbstractBalanceTransaction* trx = list->get(j);
				walletDriver->importTransaction(trx);
			}
		}
	}
}

TEST(TestPendingRequestProcessorGroup, case03_ex){
	testException<NodeQueueCommandException>();
	testException<BlockValidationException>();

}

TEST(TestPendingRequestProcessorGroup, queue01){
	StackTestPortGetter portSel;
	int port01 = portSel.allocPort();

	File projectFolder = this->env->testCaseDir();
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);

	DebugDefaultLogger logger;

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig02(param);

	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&param);

	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);

	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	node01.initBlank(0);
	node01.startNetwork(nullptr, false);

	CodablecashNodeInstance* inst = node01.getInstance();

	NodeIdentifierSource* networkKey =  NodeIdentifierSource::create(); __STP(networkKey);
	inst->startProcessors(networkKey, true);

	P2pRequestProcessor* processor = inst->getP2pRequestProcessor();
	P2pRequestQueueProcessor* queue = processor->getP2pRequestQueueProcessor();

	// blank methods
	queue->fireExecuteCommand(nullptr, nullptr);
	queue->fireOnExeptionOnSubscriber(nullptr, nullptr);
	queue->fireOnSubscribeEnds(nullptr);

	// pending NG
	{
		PubSubId* pubsubId = PubSubId::createNewId(); __STP(pubsubId);
		NopNodeCommand* command = new NopNodeCommand(); __STP(command);
		command->setValue(1);

		command->sign(networkKey);

		processor->__putPendingCommand(command, pubsubId);

		delete command->executeAsNode(nullptr, inst, false);
	}

	inst->resumeRequestProcessor();


	{
		NopNodeQueueCommand command;
		command.setNum(-1);

		PubSubId* pubsubId = PubSubId::createNewId(); __STP(pubsubId);
		processor->putQueue(pubsubId, &command);
	}

}


