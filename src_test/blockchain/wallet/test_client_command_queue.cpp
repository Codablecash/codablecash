/*
 * test_client_command_queue.cpp
 *
 *  Created on: Aug 25, 2025
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"

#include "bc_wallet_net/WalletConfig.h"
#include "bc_wallet_net/NetworkWallet.h"

#include "bc_wallet_encoder/PasswordEncoder.h"

#include "bc/DebugDefaultLogger.h"
#include "bc/CodablecashSystemParam.h"

#include "base/StackRelease.h"

#include "../utils/DebugCodablecashSystemParamSetup.h"
#include "bc_wallet_net_cmd/ClientNewTransactionCommand.h"

#include "data_history_data/TransactionTransferData.h"

#include "bc_block_body/Coinbase.h"

#include "bc_base/BalanceUnit.h"

#include "bc_block_body/CoinbaseTransaction.h"

#include "bc_wallet_net_processor/NetworkClientCommandProcessor.h"
using namespace codablecash;

TEST_GROUP(TestClientCommandQueueGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestClientCommandQueueGroup, case01) {
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"data"))

	UnicodeString pass(L"changeit");
	PasswordEncoder enc(&pass);

	DebugDefaultLogger* logger = new DebugDefaultLogger(); __STP(logger);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig02(param);

	WalletConfig walletConfig;
	{
		NetworkWallet* wallet = NetworkWallet::createNewWallet(baseDir, &pass, 0, 10, logger, &param, &walletConfig); __STP(wallet);

		NetworkClientCommandProcessor* processor  = wallet->getNetworkClientCommandProcessor();

		processor->startProcessors(false);

		int maxLoop = 1;
		for(int i = 0; i != maxLoop; ++i) {
			ClientNewTransactionCommand cmd;
			TransactionTransferData data;

			BalanceUnit amount(1L);
			Coinbase coinbase;
			coinbase.setAmount(&amount);

			CoinbaseTransaction trx;
			trx.setCoinbase(&coinbase);
			trx.setHeight(1);

			trx.build();

			data.setTransaction(&trx);
			cmd.setData(&data);

			processor->addClientCommand(&cmd);
		}

		wallet->shutdownNetwork();
		wallet->closeData();
	}
	{
		NetworkWallet* wallet = NetworkWallet::openWallet(baseDir, &pass, logger, &param, &walletConfig); __STP(wallet);

		NetworkClientCommandProcessor* processor  = wallet->getNetworkClientCommandProcessor();

		processor->startProcessors(false);

		int maxLoop = 1;
		for(int i = 0; i != maxLoop; ++i) {
			ClientNewTransactionCommand cmd;
			TransactionTransferData data;

			BalanceUnit amount(1L);
			Coinbase coinbase;
			coinbase.setAmount(&amount);

			CoinbaseTransaction trx;
			trx.setCoinbase(&coinbase);
			trx.setHeight(1);

			trx.build();

			data.setTransaction(&trx);
			cmd.setData(&data);

			processor->addClientCommand(&cmd);
		}

		wallet->shutdownNetwork();
		wallet->closeData();
	}
}

