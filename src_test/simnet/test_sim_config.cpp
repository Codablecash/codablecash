/*
 * test_sim_config.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "sim/ZoneInstanceConfig.h"

#include "bc_wallet_encoder/PasswordEncoder.h"

#include "bc_wallet_net/WalletConfig.h"
#include "bc_wallet_net/NetworkWallet.h"

#include "bc/DebugDefaultLogger.h"
#include "bc/CodablecashSystemParam.h"

#include "../blockchain/utils/DebugCodablecashSystemParamSetup.h"

#include "bc_base/AddressDescriptor.h"
#include "bc_base/BalanceUnit.h"

#include "bc_block_generator/MiningConfig.h"

#include "bc_p2p_info/P2pNodeRecord.h"

#include "bc_network/NodeIdentifierSource.h"

#include "bc_network_instance/FinalizerConfig.h"
#include "bc_network_instance/GenesisBalanceConfig.h"

#include "json/JsonHandler.h"

#include "json_object/JsonObject.h"


using namespace codablecash;

// JsonHandler

TEST_GROUP(TestSimConfigGroup) {
	TEST_SETUP(){this->env->setup();}
	TEST_TEARDOWN(){this->env->teardown();}
};

TEST(TestSimConfigGroup, case01){
	File projectFolder = this->env->testCaseDir();

	DebugDefaultLogger logger;
	logger.addSection(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE);
	logger.addSection(ISystemLogger::DEBUG_TRX_MEMPOOL_REGISTER);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig02(param);

	// wallet
	UnicodeString pass(L"changeit");
	PasswordEncoder enc(&pass);
	WalletConfig walletConfig;
	File* dirWallet00 = projectFolder.get(L"wallet00"); __STP(dirWallet00);
	NetworkWallet* wallet = NetworkWallet::createNewWallet(dirWallet00, &pass, 0, 10, &logger, &param, &walletConfig); __STP(wallet);


	ZoneInstanceConfig* conf = new ZoneInstanceConfig(); __STP(conf);

	{
		// genesis address
		AddressDescriptor* desc = wallet->getAddressDescriptor(0, 0); __STP(desc);

		GenesisBalanceConfig gconfig;
		BalanceUnit amount(1000L * 10000L * 10000L);
		gconfig.addBalance(amount, desc);
		conf->setGenesisConfig(&gconfig);
	}
	{
		// mining
		MiningConfig minerConfig;
		AddressDescriptor* desc2 = wallet->getAddressDescriptor(0, 1); __STP(desc2);
		minerConfig.setAddressDescriptor(desc2);
		minerConfig.setCalclatorType(MiningConfig::TYPE_POOL);

		minerConfig.setProtocol(P2pNodeRecord::TCP_IP_V4);
		UnicodeString host(L"127.0.0.1");
		minerConfig.setServerHost(&host);
		minerConfig.setPort(1111);

		conf->setMiningConfig(&minerConfig);
	}
	{
		// finalizer
		FinalizerConfig fconfig;
		NodeIdentifierSource* voteSource = NodeIdentifierSource::create(); __STP(voteSource);
		fconfig.setVoterSource(voteSource);
		conf->setFinalizerConfig(&fconfig);
	}

	{
		UnicodeString name(L"node.codablecash.org");
		conf->setName(&name);
		conf->setPort(18080);
		conf->setProtocol(P2pNodeRecord::TCP_IP_V4);
	}

	JsonObject* jobj = conf->toJsonObject(); __STP(jobj);
	UnicodeString* json = jobj->toString(); __STP(json);

	JsonHandler handler;
	handler.loadString(json);

	AbstractJsonObject* parsedObj = handler.parse(); __STP(parsedObj);

	ZoneInstanceConfig* cfg = ZoneInstanceConfig::fromJson(dynamic_cast<JsonObject*>(parsedObj)); __STP(cfg);

	//JsonHandler
	// JsonArrayObject


}
