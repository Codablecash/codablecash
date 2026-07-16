/*
 * test_multi_shard.cpp
 *
 *  Created on: May 16, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"
#include "test_utils/TestPortSelection.h"

#include "pubsub/CommandPublisherSetting.h"

#include "bc/DebugDefaultLogger.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_network_instance/CodablecashNetworkNodeConfig.h"
#include "bc_network_instance/GenesisBalanceConfig.h"
#include "bc_network_instance/FinalizerConfig.h"

#include "bc_block_generator/MiningConfig.h"

#include "bc_network/NodeIdentifierSource.h"

#include "bc_network_instance/CodablecashNetworkNode.h"

#include "../../p2p/p2pserver/dummy/DummyClientListner.h"
#include "../utils/ClientConnectUtils.h"
#include "bc_p2p_client/P2pClient.h"

#include "bc_wallet_net/WalletConfig.h"
#include "bc_wallet_net/NetworkWallet.h"
#include "bc_wallet_net/NetworkTransactionHandler.h"

#include "bc_wallet_encoder/PasswordEncoder.h"

#include "bc_finalizer_trx/RegisterVotePoolTransaction.h"
#include "bc_finalizer_trx/RegisterTicketTransaction.h"

#include "bc_trx/TransactionId.h"

#include "bc_wallet_net_data_management/ManagedUtxoCacheRecord.h"

#include "bc_network/NodeIdentifier.h"

#include "../wallet_util/TransactionUtxoChecker.h"
#include "bc_status_cache/BlockchainController.h"

#include "bc/CodablecashNodeInstance.h"
#include "blockchain/utils_testnet/ArrayDebugSeeder.h"
#include "blockchain/utils_testnet/TestnetInstanceWrapper.h"
#include "blockchain/utils_testnet/MultizoneTestnet.h"
#include "blockchain/utils/DebugCodablecashSystemParamSetup.h"
#include "blockchain/wallet_util/WalletDriver.h"
#include "NewShardAdder.h"
#include "NewShardValidator.h"


using namespace codablecash;


TEST_GROUP(TestMultiShardGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestMultiShardGroup, case01){
	CommandPublisherSettingStack __set(5, 300);
	TransactionUtxoChecker utxoChecker;

	StackTestPortGetter portSel;
	File projectFolder = this->env->testCaseDir();
	File* dirWallet00 = projectFolder.get(L"wallet00"); __STP(dirWallet00);
	File* dirWallet01 = projectFolder.get(L"wallet01"); __STP(dirWallet01);

	DebugDefaultLogger logger;
	logger.addSection(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE);
	logger.addSection(ISystemLogger::DEBUG_TRX_MEMPOOL_REGISTER);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig02(param);

	CodablecashNetworkNodeConfig nwconfig;
	nwconfig.setSysConfig(&param);

	// host;
	UnicodeString localhost(L"localhost");
	nwconfig.setHost(&localhost);

	MultizoneTestnet testnet(&portSel, &projectFolder, &logger, &param, &nwconfig);

	// wallet
	UnicodeString pass(L"changeit");
	PasswordEncoder enc(&pass);
	WalletConfig walletConfig;
	NetworkWallet* wallet = NetworkWallet::createNewWallet(dirWallet00, &pass, 0, 10, &logger, &param, &walletConfig); __STP(wallet);

	// wallet shard01
	NetworkWallet* wallet01 = NetworkWallet::createNewWallet(dirWallet01, &pass, 1, 10, &logger, &param, &walletConfig); __STP(wallet01);

	// genesis address
	AddressDescriptor* desc = wallet->getAddressDescriptor(0, 0); __STP(desc);// walletDriver->getAddressDesc(0, 0); __STP(desc);

	GenesisBalanceConfig gconfig;
	BalanceUnit amount(1000L * 10000L * 10000L);
	gconfig.addBalance(amount, desc);

	{
		TestnetInstanceWrapper* inst = testnet.createInstance(0, L"first");
		// genesis
		inst->setGenesisBlockConfig(&gconfig);

		// mining
		MiningConfig minerConfig;
		AddressDescriptor* desc2 = wallet->getAddressDescriptor(0, 1); __STP(desc2);
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

	{
		TestnetInstanceWrapper* inst = testnet.getInstance(0, L"first");
		NewShardAdder adder;
		adder.init(1, 0);
		adder.setHeight(6);
		inst->addIBlockGenerationListner(&adder);

		NewShardValidator shardValidator;
		inst->setShardExtendValidator(&shardValidator);


		int port01 = inst->getListeningPort();
		const NodeIdentifierSource* source = inst->getVoterIdentifierSource();

		wallet->setStakingSourceId(source, &enc);
		wallet->initNetwork(&seeder, &enc);
		// sync
		wallet->syncBlockchain(true);

		// register pool
		NetworkTransactionHandler* handler = wallet->getNetworkTransactionHandler(0); __STP(handler);
		{
			BalanceUnit fee(1L);
			handler->sendRegisterVotePoolTransaction(fee, &enc);

			const AbstractBlockchainTransaction* t = handler->getLastTransaction();
			const RegisterVotePoolTransaction* trx = dynamic_cast<const RegisterVotePoolTransaction*>(t);
			const TransactionId* trxId = trx->getTransactionId();

			// check by management account
			uint8_t type = handler->getTransactionStoreStatus(trxId);
			while(type != ManagedUtxoCacheRecord::UNFINALIZED){
				Os::usleep(200 * 1000);
				type = handler->getTransactionStoreStatus(trxId);
			}
		}

		NodeIdentifier* nodeId = nullptr;
		{
			ArrayList<NodeIdentifier>* list = handler->listStakingNodeIds();
			list->setDeleteOnExit();

			while(list->isEmpty()){
				delete list;

				Os::usleep(200 * 1000);

				list = handler->listStakingNodeIds();
				list->setDeleteOnExit();
			}
			__STP(list);

			nodeId = dynamic_cast<NodeIdentifier*>(list->get(0)->copyData());
		}
		__STP(nodeId);

		testnet.waitForBlockHeight(0, 0, 4);

		// staking ticket
		testnet.suspendMining(0);
		int maxLoop = 10;
		for(int i = 0; i != maxLoop; ++i){
			BalanceUnit fee(1L);
			BalanceUnit stakeAmount(100L);
			handler->sendRegisterTicketTransaction(nodeId, stakeAmount, fee, &enc);

			const AbstractBlockchainTransaction* t = handler->getLastTransaction();
			const RegisterTicketTransaction* trx = dynamic_cast<const RegisterTicketTransaction*>(t);

			bool checkres = utxoChecker.checkTransaction(trx);
			CHECK(checkres);

			{
				BalanceUnit unit = trx->getFeeRate();
				uint64_t am = unit.getAmount();
				CHECK(am == 1);
			}


			const TransactionId* trxId = trx->getTransactionId();
			{
				UnicodeString msg(L"Wallet TrxId : ");
				UnicodeString* trxIdStr = trxId->toString(); __STP(trxIdStr);
				msg.append(trxIdStr);

				logger.debugLog(ISystemLogger::DEBUG_TRX_MEMPOOL_REGISTER, &msg, __FILE__, __LINE__);
			}

			// check if the mempool has the TRX
			while(!inst->hasTransaction(trxId)){
				Os::usleep(200 * 1000);
			}

			// check by management account
			uint8_t type = handler->getTransactionStoreStatus(trxId);
			while(type == ManagedUtxoCacheRecord::NONE){
				Os::usleep(200 * 1000);
				type = handler->getTransactionStoreStatus(trxId);
			}

			{
				UnicodeString msg(L"---------------- ");
				logger.debugLog(ISystemLogger::DEBUG_TRX_MEMPOOL_REGISTER, &msg, __FILE__, __LINE__);
			}

		}
	}
	testnet.resumeMining(0);



	// zone 1 stand by node
	File* dirNode01 = projectFolder.get(L"node01"); __STP(dirNode01);
	CodablecashNetworkNodeConfig* config01 = new CodablecashNetworkNodeConfig(nwconfig); __STP(config01);
	int port01 = portSel.allocPort();
	config01->setPort(port01);

	CodablecashNetworkNode node01(dirNode01, config01, &logger);
	{
		// second
		node01.initBlank(1, 1); // zone 1

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

	testnet.waitForBlockHeight(0, 0, 10);
	wallet->shutdownNetwork();
}

