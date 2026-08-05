/*
 * test_ticket_transaction_wallet.cpp
 *
 *  Created on: Jun 8, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "bc_wallet_encoder/PasswordEncoder.h"

#include "bc_wallet/HdWalletSeed.h"
#include "bc_wallet/HdWallet.h"
#include "bc_wallet/WalletAccount.h"

#include "bc_base/BalanceUnit.h"
#include "bc_base/AddressDescriptor.h"

#include "bc_trx_genesis/GenesisTransaction.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc_finalizer_trx/RegisterVotePoolTransaction.h"
#include "bc_finalizer_trx/RegisterTicketTransaction.h"

#include "bc_trx/AbstractUtxoReference.h"

#include "bc_wallet_trx_repo/WalletAccountTrxRepository.h"


using namespace codablecash;

TEST_GROUP(TestTicketTransactionWalletGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

static bool balancecheck(AbstractFinalizerTransaction* trx, WalletAccount* account){
	WalletAccountTrxRepository* repo = account->getWalletAccountTrxRepository();

	BalanceUnit right = trx->getFee();
	{
		int maxLoop = trx->getUtxoSize();
		for(int i = 0; i != maxLoop; ++i){
			AbstractUtxo* utxo = trx->getUtxo(i);
			BalanceUnit unit = utxo->getAmount();

			right += unit;
		}
	}

	BalanceUnit left(0L);
	{
		int maxLoop = trx->getUtxoReferenceSize();
		for(int i = 0; i != maxLoop; ++i){
			AbstractUtxoReference* ref = trx->getUtxoReference(i);
			const UtxoId* utxoId = ref->getUtxoId();

			BalanceUtxo* utxo = repo->getBalanceUtxo(utxoId); __STP(utxo);
			BalanceUnit unit = utxo->getAmount();

			left += unit;
		}
	}

	return right.compareTo(&left) == 0;
}

TEST(TestTicketTransactionWalletGroup, case01) {
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"wallet"))

	UnicodeString pass(L"changeit");
	PasswordEncoder enc(&pass);

	{
		HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
		HdWallet* wallet = HdWallet::create(baseDir, seed, 0, &enc, 10); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);
		BalanceUnit amount(100000000L);
		GenesisTransaction* trx = account->createGenesisTransaction(amount, 5); __STP(trx);
		account->importTransaction(trx);
	}

	// register pool
	NodeIdentifierSource* source = NodeIdentifierSource::create(); __STP(source);
	NodeIdentifier nodeId = source->toNodeIdentifier();
	{
		HdWallet* wallet = HdWallet::loadWallet(baseDir, &enc); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);

		AddressDescriptor* desc = account->getReceivingAddressDescriptor(3); __STP(desc);
		RegisterVotePoolTransaction* trx = account->createRegisterVotePoolTransaction(source, BalanceUnit(1), desc, &enc); __STP(trx);

		bool vresult = trx->verify();
		CHECK(vresult);

		vresult = balancecheck(trx, account);
		CHECK(vresult);
	}

}

TEST(TestTicketTransactionWalletGroup, case02) {
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"wallet"))

	UnicodeString pass(L"changeit");
	PasswordEncoder enc(&pass);

	{
		HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
		HdWallet* wallet = HdWallet::create(baseDir, seed, 0, &enc, 10); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);

		{
			BalanceUnit amount(300L);
			GenesisTransaction* trx = account->createGenesisTransaction(amount, 5); __STP(trx);
			account->importTransaction(trx);
		}
		{
			BalanceUnit amount(400L);
			GenesisTransaction* trx = account->createGenesisTransaction(amount, 5); __STP(trx);
			account->importTransaction(trx);
		}
	}

	// register pool
	NodeIdentifierSource* source = NodeIdentifierSource::create(); __STP(source);
	NodeIdentifier nodeId = source->toNodeIdentifier();
	{
		HdWallet* wallet = HdWallet::loadWallet(baseDir, &enc); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);

		AddressDescriptor* desc = account->getReceivingAddressDescriptor(3); __STP(desc);
		RegisterVotePoolTransaction* trx = account->createRegisterVotePoolTransaction(source, BalanceUnit(1), desc, &enc); __STP(trx);

		bool vresult = trx->verify();
		CHECK(vresult);

		vresult = balancecheck(trx, account);
		CHECK(vresult);
	}
}

TEST(TestTicketTransactionWalletGroup, case03) {
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"wallet"))

	UnicodeString pass(L"changeit");
	PasswordEncoder enc(&pass);

	{
		HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
		HdWallet* wallet = HdWallet::create(baseDir, seed, 0, &enc, 10); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);
		BalanceUnit amount(400L); // (299L);
		GenesisTransaction* trx = account->createGenesisTransaction(amount, 5); __STP(trx);
		account->importTransaction(trx);
	}

	// register pool
	NodeIdentifierSource* source = NodeIdentifierSource::create(); __STP(source);
	NodeIdentifier nodeId = source->toNodeIdentifier();
	{
		HdWallet* wallet = HdWallet::loadWallet(baseDir, &enc); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);

		AddressDescriptor* desc = account->getReceivingAddressDescriptor(3); __STP(desc);
		RegisterVotePoolTransaction* trx = account->createRegisterVotePoolTransaction(source, BalanceUnit(1), desc, &enc); __STP(trx);

		bool vresult = trx->verify();
		CHECK(vresult);

		vresult = balancecheck(trx, account);
		CHECK(vresult);
	}
}

TEST(TestTicketTransactionWalletGroup, caseTicket01) {
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"wallet"))

	UnicodeString pass(L"changeit");
	PasswordEncoder enc(&pass);

	{
		HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
		HdWallet* wallet = HdWallet::create(baseDir, seed, 0, &enc, 10); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);

		{
			BalanceUnit amount(299L);
			GenesisTransaction* trx = account->createGenesisTransaction(amount, 5); __STP(trx);
			account->importTransaction(trx);
		}

		{
			BalanceUnit amount(600L);
			GenesisTransaction* trx = account->createGenesisTransaction(amount, 5); __STP(trx);
			account->importTransaction(trx);
		}
	}

	// register pool
	NodeIdentifierSource* source = NodeIdentifierSource::create(); __STP(source);
	NodeIdentifier nodeId = source->toNodeIdentifier();
	{
		HdWallet* wallet = HdWallet::loadWallet(baseDir, &enc); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);

		AddressDescriptor* desc = account->getReceivingAddressDescriptor(3); __STP(desc);
		{
			RegisterVotePoolTransaction* trx = account->createRegisterVotePoolTransaction(source, BalanceUnit(1), desc, &enc); __STP(trx);
			account->importTransaction(trx);

			bool vresult = trx->verify();
			CHECK(vresult);
		}

		{
			BalanceUnit stakeAmount(100L);
			BalanceUnit feeRate(1L);
			RegisterTicketTransaction* trx = account->createRegisterTicketTransaction(&nodeId, stakeAmount, feeRate, desc, &enc); __STP(trx);

			bool vresult = trx->verify();
			CHECK(vresult);

			vresult = balancecheck(trx, account);
			CHECK(vresult);
		}
	}
}

TEST(TestTicketTransactionWalletGroup, caseTicket02) {
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"wallet"))

	UnicodeString pass(L"changeit");
	PasswordEncoder enc(&pass);

	{
		HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
		HdWallet* wallet = HdWallet::create(baseDir, seed, 0, &enc, 10); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);

		{
			BalanceUnit amount(296L);
			GenesisTransaction* trx = account->createGenesisTransaction(amount, 5); __STP(trx);
			account->importTransaction(trx);
		}

		{
			BalanceUnit amount(400L);
			GenesisTransaction* trx = account->createGenesisTransaction(amount, 5); __STP(trx);
			account->importTransaction(trx);
		}
		{
			BalanceUnit amount(400L);
			GenesisTransaction* trx = account->createGenesisTransaction(amount, 5); __STP(trx);
			account->importTransaction(trx);
		}
	}

	// register pool
	NodeIdentifierSource* source = NodeIdentifierSource::create(); __STP(source);
	NodeIdentifier nodeId = source->toNodeIdentifier();
	{
		HdWallet* wallet = HdWallet::loadWallet(baseDir, &enc); __STP(wallet);
		WalletAccount* account = wallet->getAccount(0);

		AddressDescriptor* desc = account->getReceivingAddressDescriptor(3); __STP(desc);
		{
			RegisterVotePoolTransaction* trx = account->createRegisterVotePoolTransaction(source, BalanceUnit(1), desc, &enc); __STP(trx);
			account->importTransaction(trx);

			bool vresult = trx->verify();
			CHECK(vresult);
		}

		{
			BalanceUnit stakeAmount(100L);
			BalanceUnit feeRate(1L);
			RegisterTicketTransaction* trx = account->createRegisterTicketTransaction(&nodeId, stakeAmount, feeRate, desc, &enc); __STP(trx);

			bool vresult = trx->verify();
			CHECK(vresult);

			vresult = balancecheck(trx, account);
			CHECK(vresult);
		}
	}
}
