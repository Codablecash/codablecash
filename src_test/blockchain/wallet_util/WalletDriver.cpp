/*
 * WalletDriver.cpp
 *
 *  Created on: 2023/02/26
 *      Author: iizuka
 */

#include "WalletDriver.h"

#include "base/StackRelease.h"

#include "base_io/File.h"

#include "bc_wallet/HdWallet.h"
#include "bc_wallet/HdWalletSeed.h"
#include "bc_wallet/WalletAccount.h"

#include "bc_wallet_encoder/PasswordEncoder.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_trx/NopInterChainCommunicationTransaction.h"

#include "bc_trx_balance/BalanceTransferTransaction.h"

#include "bc_trx_genesis/GenesisTransaction.h"

#include "bc_finalizer_trx/RegisterVotePoolTransaction.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "../utils/TransactionWaiter.h"

#include "bc_p2p_client/StackClientListnerRemover.h"
#include "bc_p2p_client/P2pClient.h"

#include "bc_p2p_cmd_client/SendTransactionClientCommand.h"

#include "pubsub_cmd/AbstractCommandResponse.h"

#include "transaction/NopSmartcontractTransaction.h"

#include "bc_finalizer_trx/RegisterTicketTransaction.h"

#include "osenv/funcs.h"

#include "bc_memorypool/MemoryPool.h"

namespace codablecash {

WalletDriver::WalletDriver(uint16_t zone, const File* baseDir, const wchar_t* pass) {
	this->zone = zone;
	this->baseDir = new File(*baseDir);

	UnicodeString password(pass);
	this->enc = new PasswordEncoder(&password);

	this->stakeSource = NodeIdentifierSource::create();
}

WalletDriver::~WalletDriver() {
	this->list.deleteElements();
	delete this->enc;
	delete this->baseDir;
	delete this->stakeSource;
}

void WalletDriver::init(int numWallets) {
	for(int i = 0; i != numWallets; ++i){
		UnicodeString seg(L"");
		seg.append((int64_t)i);

		File* f = this->baseDir->get(&seg); __STP(f);
		HdWallet* wallet = newWallet(f);
		this->list.addElement(wallet);
	}
}

void WalletDriver::initGenesis(int index, uint64_t defaultBalance) {
	HdWallet* wallet = this->list.get(index);
	WalletAccount* account = wallet->getAccount(0);

	GenesisTransaction* trx = createGenesisTransaction(index, defaultBalance); __STP(trx);
	account->importTransaction(trx);
}

GenesisTransaction* WalletDriver::createGenesisTransaction(int index,	uint64_t defaultBalance) {
	HdWallet* wallet = this->list.get(index);
	WalletAccount* account = wallet->getAccount(0);

	BalanceUnit amount(defaultBalance);
	GenesisTransaction* trx = account->createGenesisTransaction(amount, 0);

	return trx;
}

HdWallet* WalletDriver::newWallet(const File *baseDir) {
	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	HdWallet* wallet = HdWallet::create(this->baseDir, seed, this->zone, this->enc, 256);

	return wallet;
}

bool WalletDriver::transfer(int srcIndex, int destIndex, int destAddressIndex, const BalanceUnit amount
		, const BalanceUnit feeRate, bool feeIncluded) {
	HdWallet* wallet = this->list.get(srcIndex);
	WalletAccount* account = wallet->getAccount(0);

	HdWallet* walletDest = this->list.get(destIndex);
	WalletAccount* accountDest = walletDest->getAccount(0);
	AddressDescriptor* destDesc = accountDest->getReceivingAddressDescriptor(destAddressIndex); __STP(destDesc);

	BalanceTransferTransaction* trx = account->createBalanceTransferTransaction(destDesc, amount, feeRate, feeIncluded, this->enc); __STP(trx);
	bool verified = trx->verify();

	account->importTransaction(trx);
	accountDest->importTransaction(trx);

	return verified;
}

BalanceTransferTransaction* WalletDriver::createBalanceTransferTransaction(
		int srcIndex, int destIndex, int destAddressIndex,
		const BalanceUnit amount, const BalanceUnit feeRate, bool feeIncluded) {
	HdWallet* wallet = this->list.get(srcIndex);
	WalletAccount* account = wallet->getAccount(0);

	HdWallet* walletDest = this->list.get(destIndex);
	WalletAccount* accountDest = walletDest->getAccount(0);
	AddressDescriptor* destDesc = accountDest->getReceivingAddressDescriptor(destAddressIndex); __STP(destDesc);

	BalanceTransferTransaction* trx = account->createBalanceTransferTransaction(destDesc, amount, feeRate, feeIncluded, this->enc);
	bool verified = trx->verify();
	assert(verified);

	return trx;
}

BalanceUnit WalletDriver::getTotalBalance(int srcIndex) const noexcept {
	HdWallet* wallet = this->list.get(srcIndex);
	WalletAccount* account = wallet->getAccount(0);

	return account->getTotalAmount();
}

RegisterVotePoolTransaction* WalletDriver::createRegisterVotePoolTransaction(
		int srcIndex, const NodeIdentifierSource *source, const BalanceUnit feeRate) {
	HdWallet* wallet = this->list.get(srcIndex);
	WalletAccount* account = wallet->getAccount(0);

	AddressDescriptor* destDesc = account->getReceivingAddressDescriptor(0); __STP(destDesc);

	RegisterVotePoolTransaction* trx = account->createRegisterVotePoolTransaction(source, feeRate, destDesc, this->enc);

	return trx;
}

AbstractCommandResponse* WalletDriver::registerVotingNode(P2pClient *client) {
	NodeIdentifier nodeId = this->stakeSource->toNodeIdentifier();
	{
		RegisterVotePoolTransaction* trx = createRegisterVotePoolTransaction(0, this->stakeSource, BalanceUnit(1L)); __STP(trx);

		importTransaction(trx);

		const TransactionId* trxId = trx->getTransactionId();
		TransactionWaiter waiter(trxId);
		StackClientListnerRemover remover(client, &waiter);

		SendTransactionClientCommand cmd;
		cmd.setNodeIdentifier(&nodeId);
		cmd.setTransaction(trx);

		cmd.sign(client->getNodeIdentifierSource());

		AbstractCommandResponse* response = client->sendCommnad(&cmd); __STP(response);

		return __STP_MV(response);
	}
}


AbstractCommandResponse* WalletDriver::registerTicket(P2pClient *client, const BalanceUnit &stakeAmount) {
	return registerTicket(client, stakeAmount, nullptr);
}

AbstractCommandResponse* WalletDriver::registerTicket(P2pClient *client, const BalanceUnit &stakeAmount, MemoryPool *mempool) {
	NodeIdentifier nodeId = this->stakeSource->toNodeIdentifier();
	{
		BalanceUnit feeRate(100L);
		RegisterTicketTransaction* trx = createRegisterTicketTransaction(0, 0, 0, &nodeId, stakeAmount, feeRate); __STP(trx);

		importTransaction(trx);

		SendTransactionClientCommand cmd;
		cmd.setTransaction(trx);

		cmd.sign(client->getNodeIdentifierSource());

		AbstractCommandResponse* response = client->sendCommnad(&cmd); __STP(response);

		if(mempool != nullptr){
			bool exists = false;
			const TransactionId* trxId = trx->getTransactionId();

			while(!exists){
				AbstractControlTransaction* regTrx = mempool->getControlTransaction(trxId); __STP(regTrx);

				exists = (regTrx != nullptr);
				if(!exists){
					Os::usleep(100*1000);
				}
			}
		}

		return __STP_MV(response);
	}
}

AbstractCommandResponse* WalletDriver::sendNopSmartcontractTrx(P2pClient *client) {
	NopSmartcontractTransaction trx;

	SendTransactionClientCommand cmd;
	cmd.setTransaction(&trx);

	cmd.sign(client->getNodeIdentifierSource());

	AbstractCommandResponse* response = client->sendCommnad(&cmd); __STP(response);

	return __STP_MV(response);
}

AbstractCommandResponse* WalletDriver::sendNopIccTrx(P2pClient *client) {
	NopInterChainCommunicationTransaction trx;

	SendTransactionClientCommand cmd;
	cmd.setTransaction(&trx);

	cmd.sign(client->getNodeIdentifierSource());

	AbstractCommandResponse* response = client->sendCommnad(&cmd); __STP(response);

	return __STP_MV(response);
}


AbstractCommandResponse* WalletDriver::transfer(int srcIndex, int destIndex, int destAddressIndex, const BalanceUnit& amount,
		const BalanceUnit& feeRate, bool feeIncluded, P2pClient *client) {
	return transfer(srcIndex, destIndex, destAddressIndex, amount, feeRate, feeIncluded, client, nullptr);
}

AbstractCommandResponse* WalletDriver::transfer(int srcIndex, int destIndex,	int destAddressIndex, const BalanceUnit& amount,
		const BalanceUnit& feeRate, bool feeIncluded, P2pClient *client, MemoryPool *mempool) {
	BalanceTransferTransaction* trx = createBalanceTransferTransaction(srcIndex, destIndex, destAddressIndex, amount, feeRate, feeIncluded); __STP(trx);

	importTransaction(trx);

	SendTransactionClientCommand cmd;
	cmd.setTransaction(trx);

	cmd.sign(client->getNodeIdentifierSource());

	AbstractCommandResponse* response = client->sendCommnad(&cmd); __STP(response);

	if(mempool != nullptr){
		bool exists = false;
		const TransactionId* trxId = trx->getTransactionId();

		while(!exists){
			AbstractBalanceTransaction* regTrx = mempool->getBalanceTransaction(trxId); __STP(regTrx);

			exists = (regTrx != nullptr);
			if(!exists){
				Os::usleep(100*1000);
			}
		}
	}

	return __STP_MV(response);
}

RegisterTicketTransaction* WalletDriver::createRegisterTicketTransaction(
		int srcIndex, int destIndex, int destAddressIndex, const NodeIdentifier *nodeId
		, const BalanceUnit& stakeAmount, const BalanceUnit& feeRate) {
	HdWallet* wallet = this->list.get(srcIndex);
	WalletAccount* account = wallet->getAccount(0);

	HdWallet* walletDest = this->list.get(destIndex);
	WalletAccount* accountDest = walletDest->getAccount(0);
	AddressDescriptor* destDesc = accountDest->getReceivingAddressDescriptor(destAddressIndex); __STP(destDesc);

	RegisterTicketTransaction* trx = account->createRegisterTicketTransaction(nodeId, stakeAmount, feeRate, destDesc, this->enc);
	return trx;
}

/**
 * import transaction to all wallets
 * @param trx
 */
void WalletDriver::importTransaction(const AbstractBlockchainTransaction *trx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		HdWallet* wallet = this->list.get(i);
		WalletAccount* account = wallet->getAccount(0);

		account->importTransaction(trx);
	}
}

AddressDescriptor* WalletDriver::getAddressDesc(int destIndex,	int destAddressIndex) const {
	HdWallet* walletDest = this->list.get(destIndex);
	WalletAccount* accountDest = walletDest->getAccount(0);
	AddressDescriptor* destDesc = accountDest->getReceivingAddressDescriptor(destAddressIndex); __STP(destDesc);

	return __STP_MV(destDesc);
}

WalletAccount* WalletDriver::getAccount(int walletIndex, int accountIndex) {
	HdWallet* walletDest = this->list.get(walletIndex);
	WalletAccount* account = walletDest->getAccount(accountIndex);

	return account;
}

void WalletDriver::setStakeSource(const NodeIdentifierSource *stakeSource) noexcept {
	delete this->stakeSource;
	this->stakeSource = dynamic_cast<NodeIdentifierSource*>(stakeSource->copyData());
}

} /* namespace codablecash */
