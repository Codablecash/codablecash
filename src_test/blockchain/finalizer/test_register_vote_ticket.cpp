/*
 * test_register_vote_ticket.cpp
 *
 *  Created on: 2023/04/26
 *      Author: iizuka
 */
#include "bc/CodablecashSystemParam.h"

#include "test_utils/t_macros.h"

#include "../utils/InstanceDriver.h"
#include "../utils/DebugCodablecashSystemParamSetup.h"

#include "bc_block_generator/MiningConfig.h"

#include "bc_base/BalanceUnit.h"

#include "bc_wallet/WalletAccount.h"

#include "../wallet_util/WalletDriver.h"

#include "bc_wallet_trx_repo/WalletAccountTrxRepository.h"
#include "bc_wallet_trx_repo/WalletAccountUtxoRepository.h"

#include "btree/BtreeScanner.h"

#include "bc_base_utxo_index/UtxoIdKey.h"

#include "bc/CodablecashNodeInstance.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc/DebugDefaultLogger.h"

#include "bc_trx/UtxoId.h"
using namespace codablecash;

TEST_GROUP(TestRegisterVoteTicketGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestRegisterVoteTicketGroup, case01){
	File projectFolder = this->env->testCaseDir();
	InstanceDriver driver(&projectFolder);

	DebugDefaultLogger* logger = driver.getLogger();
	logger->addSection(ISystemLogger::DEBUG_UTXO_ID);

	CodablecashSystemParam param;
	DebugCodablecashSystemParamSetup::setupConfig01(param);

	driver.initWallet(1);
	driver.initInstance(&param);

	MiningConfig mconfig;
	driver.startMiner(&mconfig);

	driver.startFinalize();

	driver.initGenesisBalance(0, 10000000000);

	driver.registerVotingNode();

	driver.waitUntilHeight(4);
	{
		CodablecashNodeInstance* inst = driver.getInstance();
		BlockchainController* ctrl = inst->getController();

		IStatusCacheContext* context = ctrl->getStatusCacheContext(0); __STP(context);


		WalletAccount* account = driver.getWallet()->getAccount(0, 0);
		WalletAccountTrxRepository* repo = account->getWalletAccountTrxRepository();
		WalletAccountUtxoRepository* utxoRepo = repo->getUtxoRepo();

		BtreeScanner* scanner = utxoRepo->getScanner(); __STP(scanner);
		while(scanner->hasNext()){
			const IBlockObject* obj = scanner->next();
			const AbstractBtreeKey* key = scanner->nextKey();


			const UtxoIdKey* utxoIdkry = dynamic_cast<const UtxoIdKey*>(key);
			const UtxoId* utxoId = utxoIdkry->getUtxoId();

			UnicodeString* utxoIdstr = utxoId->toString(); __STP(utxoIdstr);
			UnicodeString debugstr(L"Wallet utxo id : ");
			debugstr.append(utxoIdstr);
			logger->debugLog(ISystemLogger::DEBUG_UTXO_ID, &debugstr, __FILE__, __LINE__);

			AbstractUtxo* utxo = context->getUtxo(utxoId); __STP(utxo);
			CHECK(utxo != nullptr);
		}
	}


	{
		BalanceUnit stakeAmount(1000L);
		driver.registerTicket(stakeAmount);
	}
	driver.waitUntilHeight(5);


}
