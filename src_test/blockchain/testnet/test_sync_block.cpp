/*
 * test_sync_block.cpp
 *
 *  Created on: 2023/12/07
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../../test_utils/TestPortSelection.h"
#include "../utils/DebugCodablecashConfigSetup.h"
#include "../wallet_util/WalletDriver.h"
#include "../../p2p/p2pserver/dummy/DummyClientListner.h"
#include "../utils/ClientConnectUtils.h"
#include "../utils/InstanceDriver.h"

#include "bc/DebugDefaultLogger.h"
#include "bc/CodablecashConfig.h"
#include "bc/CodablecashNodeInstance.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc_network_instance/CodablecashNetworkNodeConfig.h"
#include "bc_network_instance/GenesisBalanceConfig.h"
#include "bc_network_instance/ArrayNetworkSheeder.h"
#include "bc_network_instance/CodablecashNetworkNode.h"
#include "bc_network_instance/FinalizerConfig.h"

#include "bc_block_generator/MiningConfig.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_p2p_info/P2pNodeRecord.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_block/Block.h"

#include "bc_block_body/BlockBody.h"

#include "bc_p2p_client/P2pClient.h"

#include "base/StackRelease.h"

#include "pubsub_cmd/AbstractCommandResponse.h"

#include "base/UnicodeString.h"

#include "bc_memorypool/MemoryPool.h"

#include "base_thread/StackUnlocker.h"


using alinous::Os;

#include "osenv/funcs.h"
using namespace codablecash;

TEST_GROUP(TestSyncBlockGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestSyncBlockGroup, case01){
	StackTestPortGetter portSel;
	int port01 = portSel.allocPort();
	int port02 = portSel.allocPort();

	File projectFolder = this->env->testCaseDir();
	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);
	File* dirNode02 = projectFolder.get(L"node02"); __STP(dirNode02);

	DebugDefaultLogger logger;

	CodablecashConfig config;
	DebugCodablecashConfigSetup::setupConfig02(config);

	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&config);

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

		{
			FinalizerConfig finalizerConfig;

			NodeIdentifierSource* voterKey =  NodeIdentifierSource::create(); __STP(voterKey);
			finalizerConfig.setVoterSource(voterKey);
			config01->setFinalizerConfig(&finalizerConfig);
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

		// node01.startMiner();
		node01.startStakePool();
		node01.startMiner();

		// setup seeder
		{
			uint16_t zone = 0;
			const NodeIdentifierSource* source = config01->getNetworkKey();
			NodeIdentifier nodeId = source->toNodeIdentifier();
			const wchar_t* host = L"::1";
			int port = config01->getPort();

			P2pNodeRecord* rec = P2pNodeRecord::createIpV6Record(zone, &nodeId, host, port); __STP(rec);
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

		// sync Stake source
		const NodeIdentifierSource* stakeSource = node01.getVoterSource();
		walletDriver->setStakeSource(stakeSource);
	}

	///// put mempool trx
	DummyClientListner listner;
	int trxCount = 0;
	{
		UnicodeString strLocal(L"::1");
		P2pClient* client = ClientConnectUtils::connect(&strLocal, port01, 0, &logger); __STP(client);
		client->addListner(&listner);

		{
			AbstractCommandResponse* res = walletDriver->registerVotingNode(client); __STP(res);
			UnicodeString* str = res->toString(); __STP(str);

			CHECK(res->getType() == AbstractCommandResponse::TYPE_RES_OK);
			trxCount++;
		}

		// check voter is registered
		while(true){
			BlockchainController* ctrl = node01.getInstance()->getController();
			NodeIdentifierSource* stakeSource = walletDriver->getStakeSource();
			NodeIdentifier nodeId = stakeSource->toNodeIdentifier();

			const VoterEntry* entry = ctrl->getVoterEntry(&nodeId);
			if(entry != nullptr){
				break;
			}

			Os::usleep(30 * 1000);
		}

		node01.suspendMining();

		MemoryPool* mempool = node01.getInstance()->getMemoryPool();

		int maxLoop = 40;
		for(int i = 0; i != maxLoop; ++i) {
			BalanceUnit stakeAmount(1000L);

			AbstractCommandResponse* res = walletDriver->registerTicket(client, stakeAmount, mempool); __STP(res);
			UnicodeString* str = res->toString(); __STP(str);

			CHECK(res->getType() == AbstractCommandResponse::TYPE_RES_OK);

			trxCount++;
		}

		{
			AbstractCommandResponse* res = walletDriver->transfer(0, 0, 1
					, BalanceUnit(2000), BalanceUnit(1), false, client, mempool); __STP(res);
			UnicodeString* str = res->toString(); __STP(str);

			CHECK(res->getType() == AbstractCommandResponse::TYPE_RES_OK);
			trxCount++;
		}

		{
			for(int i = 0; i != 4; ++i){
				AbstractCommandResponse* res = walletDriver->sendNopIccTrx(client); __STP(res);
				UnicodeString* str = res->toString(); __STP(str);

				CHECK(res->getType() == AbstractCommandResponse::TYPE_RES_OK);
				trxCount++;
			}
		}
		{
			for(int i = 0; i != 4; ++i){
				AbstractCommandResponse* res = walletDriver->sendNopSmartcontractTrx(client); __STP(res);
				UnicodeString* str = res->toString(); __STP(str);

				CHECK(res->getType() == AbstractCommandResponse::TYPE_RES_OK);
				trxCount++;
			}
		}

	}

	listner.waitUntilCount(trxCount);

	node01.resumeMining();

	// wait for mining
	InstanceDriver::waitUntilFinalizedHeight(3, node01.getInstance());
	node01.suspendMining();

	///////////////
	{
		config02->setPort(port02);
	}

	CodablecashNetworkNode node02(dirNode02, config02, &logger);
	{
		// second
		node02.initBlank(0); // zone 0

		// after init
		node02.startNetwork(&seeder, true);
		node02.setNodeName(L"02");

		// sync
		node02.syncNetwork();
	}
}

