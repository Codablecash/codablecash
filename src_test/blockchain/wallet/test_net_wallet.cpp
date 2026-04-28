/*
 * test_net_wallet.cpp
 *
 *  Created on: Feb 14, 2025
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"

#include "base_io/File.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_wallet_net/NetworkWallet.h"
#include "bc_wallet_net/WalletConfig.h"
#include "bc_wallet_net/NetworkTransactionHandler.h"

#include "bc_wallet/HdWalletSeed.h"

#include "bc_network_instance/INetworkSeeder.h"
#include "../utils_testnet/MultizoneTestnet.h"
#include "../utils_testnet/ArrayDebugSeeder.h"
#include "../utils_testnet/IDebugSeeder.h"
#include "../utils_testnet/TestnetInstanceWrapper.h"
#include "../../test_utils/TestPortSelection.h"

#include "pubsub/CommandPublisherSetting.h"

#include "bc/DebugDefaultLogger.h"
#include "bc/CodablecashSystemParam.h"

#include "../utils/DebugCodablecashSystemParamSetup.h"

#include "bc_base/BalanceUnit.h"

#include "bc_block_generator/MiningConfig.h"

#include "bc_network_instance/FinalizerConfig.h"
#include "bc_network_instance/GenesisBalanceConfig.h"
#include "bc_network_instance/CodablecashNetworkNodeConfig.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc_wallet_encoder/PasswordEncoder.h"

#include "osenv/funcs.h"

#include "bc_finalizer_trx/RegisterTicketTransaction.h"

#include "../utils_testnet/ZoneTestnet.h"
#include "bc_finalizer_trx/RegisterVotePoolTransaction.h"

#include "bc_wallet_net_data_management/ManagedUtxoCacheRecord.h"

#include "bc_trx/NopInterChainCommunicationTransaction.h"

#include "bc_trx_balance/BalanceUtxo.h"

#include "transaction/NopSmartcontractTransaction.h"
#include "setup/TestnetSetupper01.h"


using namespace codablecash;

class TesGrouptParams {
public:
	TesGrouptParams(){
		this->rootSeed = nullptr;
		this->seeder = new ArrayDebugSeeder();
		this->logger = new DebugDefaultLogger();
		this->config = nullptr;
		this->source = nullptr;
		this->testnet = nullptr;
	}
	~TesGrouptParams(){
		delete this->rootSeed;
		delete this->seeder;
		delete this->logger;
		delete this->config;
		delete this->source;
		this->testnet = nullptr;
	}

	HdWalletSeed* rootSeed;
	IDebugSeeder* seeder;
	DebugDefaultLogger* logger;
	CodablecashSystemParam *config;
	NodeIdentifierSource *source;
	MultizoneTestnet* testnet;
};

TEST_GROUP(TestNetWalletGroup) {
	TEST_SETUP(){
		env->setup();

		CommandPublisherSettingStack __set(5, 300);

		this->portSel = new StackTestPortGetter();
		this->params = new TesGrouptParams();
		this->params->logger->addSection(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE);

		DebugDefaultLogger* logger = this->params->logger;
		logger->addSection(ISystemLogger::DEBUG_CHAIN_HEAD_DETECT);


		this->env->getTestGroup()->setParam(this->params);

		File projectFolder = this->env->testCaseDir();
		_ST(File, baseDir, projectFolder.get(L"wallet_before"))
		_ST(File, baseTestnetDir, projectFolder.get(L"testnet"))

		UnicodeString pass(L"changeit");
		PasswordEncoder enc(&pass);

		CodablecashSystemParam param;
		DebugCodablecashSystemParamSetup::setupConfig02(param);

		WalletConfig walletConfig;
		NetworkWallet* wallet = NetworkWallet::createNewWallet(baseDir, &pass, 0, 10, logger, &param, &walletConfig); __STP(wallet);
		this->params->rootSeed = wallet->getRootSeed(&enc);
		this->params->config = new CodablecashSystemParam(param);

		this->testnet = TestnetSetupper01::setup2NodeZone0(wallet, 1000L * 10000L * 10000L, params->seeder, this->portSel, baseTestnetDir, params->logger);
		this->params->testnet = this->testnet;

		// staking source
		{
			TestnetInstanceWrapper* wr = this->testnet->getZone(0)->getInstance(0);
			const FinalizerConfig* fconfig = wr->getFinalizerConfig();

			this->params->source = dynamic_cast<NodeIdentifierSource*>(fconfig->getVoterSource()->copyData());
		}

	}
	TEST_TEARDOWN(){
		delete this->testnet;
		delete this->params;
		delete this->portSel;

		env->teardown();
	}

public:
	TesGrouptParams* params;
	StackTestPortGetter* portSel;
	MultizoneTestnet* testnet;
};

void waitForBlocks(MultizoneTestnet* testnet, int num) {
	ZoneTestnet* zoneNet = testnet->getZone(0);
	TestnetInstanceWrapper* instw = zoneNet->getInstance(0);

	uint64_t height = instw->getHeight();
	uint64_t targetHeight = height + num;

	while(height < targetHeight){
		Os::usleep(300 * 1000);
		height = instw->getHeight();
	}
}

void waitForFinalizedBlock(NetworkTransactionHandler* handler, uint64_t height) {
	uint64_t finalizedHeight = handler->getFinalizedHeight();

	while(finalizedHeight < height){
		Os::usleep(300 * 1000);
		finalizedHeight = handler->getFinalizedHeight();
	}
}

TEST(TestNetWalletGroup, case01){
	TesGrouptParams* params = (TesGrouptParams*)(this->env->getTestGroup()->getParam());
	const HdWalletSeed* rootSeed = params->rootSeed;
	INetworkSeeder* seeder = params->seeder;
	const CodablecashSystemParam* config = params->config;
	const NodeIdentifierSource* source = params->source;
	MultizoneTestnet* testnet = params->testnet;

	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"wallet"))

	DebugDefaultLogger* logger = params->logger;

	WalletConfig walletConfig;
	UnicodeString pass(L"changeit");
	NetworkWallet* wallet = NetworkWallet::resotreWallet(baseDir, &pass, 0, rootSeed, 10, logger, config, &walletConfig); __STP(wallet);

	PasswordEncoder enc(&pass);
	wallet->setStakingSourceId(source, &enc);
	wallet->initNetwork(seeder, &enc);

	// sync
	wallet->syncBlockchain();
	waitForBlocks(testnet, 2);
	wallet->resumeNetwork();


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

	waitForBlocks(testnet, 2);

	testnet->suspendMining(0);

	int maxLoop = 20;
	for(int i = 0; i != maxLoop; ++i){
		BalanceUnit fee(1L);
		BalanceUnit stakeAmount(100L);
		handler->sendRegisterTicketTransaction(nodeId, stakeAmount, fee, &enc);

		const AbstractBlockchainTransaction* t = handler->getLastTransaction();
		const RegisterTicketTransaction* trx = dynamic_cast<const RegisterTicketTransaction*>(t);

		const TransactionId* trxId = trx->getTransactionId();

		// check by management account
		uint8_t type = handler->getTransactionStoreStatus(trxId);
		while(type == ManagedUtxoCacheRecord::NONE){
			Os::usleep(200 * 1000);
			type = handler->getTransactionStoreStatus(trxId);
		}
	}

	// dummy transactions
	{
		NopInterChainCommunicationTransaction nopTrx;

		AddressDescriptor* desc = wallet->getAddressDescriptor(0, 0); __STP(desc);
		BalanceUtxo utxo;
		utxo.setAddress(desc);
		nopTrx.addutxo(&utxo);

		nopTrx.build();
		handler->sendInterChainCommunicationTransaction(&nopTrx);

		const TransactionId* trxId = nopTrx.getTransactionId();
		// check by management account
		uint8_t type = handler->getTransactionStoreStatus(trxId);
		while(type == ManagedUtxoCacheRecord::NONE){
			Os::usleep(200 * 1000);
			type = handler->getTransactionStoreStatus(trxId);
		}
	}
	{
		NopSmartcontractTransaction nopTrx;

		AddressDescriptor* desc = wallet->getAddressDescriptor(0, 0); __STP(desc);
		BalanceUtxo utxo;
		utxo.setAddress(desc);
		nopTrx.addutxo(&utxo);

		nopTrx.build();
		handler->sendSmartcontractTansaction(&nopTrx);

		const TransactionId* trxId = nopTrx.getTransactionId();
		// check by management account
		uint8_t type = handler->getTransactionStoreStatus(trxId);
		while(type == ManagedUtxoCacheRecord::NONE){
			Os::usleep(200 * 1000);
			type = handler->getTransactionStoreStatus(trxId);
		}
	}

	{
		ArrayList<AbstractBlockchainTransaction>* memret = handler->fetchMempoolTransactions(); __STP(memret);
		memret->setDeleteOnExit();
	}

	testnet->resumeMining(0);


	waitForFinalizedBlock(handler, 7);
	waitForBlocks(testnet, 2);
}
