/*
 * WalletAccount.cpp
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#include "bc_wallet/WalletAccount.h"
#include "bc_wallet/ReceivingAddressStore.h"
#include "bc_wallet/ChangeAddressStore.h"
#include "bc_wallet/HdWalletSeed.h"
#include "bc_wallet/HdWalleMuSigSignerProvidor.h"

#include "bc_wallet_encoder/IWalletDataEncoder.h"

#include "base/StackRelease.h"

#include "bc_wallet_trx_repo/WalletAccountTrxRepository.h"

#include "base_io/File.h"

#include "bc_base/BalanceUnit.h"
#include "bc_base/AddressDescriptor.h"

#include "bc_trx/AbstractBlockchainTransaction.h"

#include "bc_trx_genesis/GenesisTransaction.h"

#include "bc_trx_balance/BalanceUtxo.h"
#include "bc_trx_balance/BalanceTransferTransaction.h"
#include "bc_trx_balance/BalanceUtxoReference.h"

#include "bc_base_conf_store/StatusStore.h"

#include "musig/IMuSigSigner.h"

#include "ecda/Secp256k1Point.h"
#include "ecda/Secp256k1CompressedPoint.h"

#include "bc_wallet_trx/GenesisTransactionHandler.h"
#include "bc_wallet_trx/BalanceTransactionWalletHandler.h"
#include "bc_wallet_trx/RegisterVotePoolTransactionWalletHandler.h"
#include "bc_wallet_trx/RegisterTicketTransactionWalletHandler.h"
#include "bc_wallet_trx/CoinbaseTransactionWalletHandler.h"
#include "bc_wallet_trx/VoteBlockTransactionWalletHandler.h"
#include "bc_wallet_trx/RevokeMissedTicketWalletHandler.h"
#include "bc_wallet_trx/RevokeMissVotedTicketWalletHandler.h"
#include "bc_wallet_trx/StakeBaseTransactionWalletHandler.h"

#include "bc_network/NodeIdentifier.h"

#include "btree/BtreeScanner.h"

#include "bc_base_trx_index/TransactionData.h"

#include "bc_wallet_filter/BloomFilter1024.h"

#include "filestore_block/IBlockObject.h"

#include "bc_wallet_trx_base/HdWalletAccountTrxBuilderContext.h"

#include "bc/ExceptionThrower.h"

#include "bc_p2p_cmd_node/InvalidTransactionException.h"

#include "bc_wallet_trx/IccNopTransactionWalletHandler.h"

#include "bc_wallet_trx/SmartcontractNopTransactionWalletHandler.h"
namespace codablecash {

WalletAccount::WalletAccount(const File* accountBaseDir) : AbstractWalletAccount(0, 0) {
	this->accountBaseDir = new File(*accountBaseDir);

	this->receivingAddresses = nullptr;
	this->changeAddresses = nullptr;
	this->encryptedSeed = nullptr;
	this->trxRepo = nullptr;

	this->store = new StatusStore(accountBaseDir, WalletAccount::STORE_NAME);
	this->bloomFilter = nullptr;
}

WalletAccount::~WalletAccount() {
	delete this->store;

	delete this->accountBaseDir;

	delete this->receivingAddresses;
	delete this->changeAddresses;
	delete this->encryptedSeed;
	delete this->trxRepo;
	delete this->bloomFilter;
}

void WalletAccount::setEncryptedSeed(HdWalletSeed *encrypted) noexcept {
	delete this->encryptedSeed;
	this->encryptedSeed = encrypted;
}

WalletAccount* WalletAccount::newAccount(const File* baseDir, const HdWalletSeed *rootSeed, int accountIndex, uint16_t zone
		, const IWalletDataEncoder* encoder, int maxAddress) {
	UnicodeString seg(L"");
	seg.append((int64_t)accountIndex);
	File* accountBase = baseDir->get(&seg); __STP(accountBase);

	WalletAccount* account = new WalletAccount(accountBase);
	account->accountIndex = accountIndex;
	account->zone = zone;

	HdWalletSeed* seed = rootSeed->indexedSeed(accountIndex); __STP(seed);
	HdWalletSeed* encryptedSeed = seed->encodedSeed(encoder);
	account->setEncryptedSeed(encryptedSeed);

	account->initAddressStores(encoder, maxAddress);

	account->initTransactionRepository();

	account->save();

	return account;
}

WalletAccount* WalletAccount::loadAccount(const File *baseDir, int accountIndex, const IWalletDataEncoder *encoder) {
	UnicodeString seg(L"");
	seg.append((int64_t)accountIndex);
	File* accountBase = baseDir->get(&seg); __STP(accountBase);

	WalletAccount* account = new WalletAccount(accountBase);
	account->load(encoder);

	return account;
}


void WalletAccount::initTransactionRepository() {
	this->trxRepo = new WalletAccountTrxRepository(this);
	this->trxRepo->init();
	this->trxRepo->open();
}


void WalletAccount::initAddressStores(const IWalletDataEncoder *encoder, int maxAddress) {
	HdWalletSeed* accountRootSeed = encoder->decode(this->encryptedSeed); __STP(accountRootSeed);

	initReceivingAddressStore(accountRootSeed, maxAddress, encoder);
	initChangeAddressStore(accountRootSeed, encoder, maxAddress);
}

void WalletAccount::initReceivingAddressStore(const HdWalletSeed *rootAccountSeed, int maxAddressCount, const IWalletDataEncoder *encoder) {
	this->receivingAddresses = new ReceivingAddressStore(this->zone, maxAddressCount, this->accountBaseDir);

	HdWalletSeed* s = rootAccountSeed->indexedSeed(1); __STP(s);
	HdWalletSeed* encrypted = encoder->encode(s);

	this->receivingAddresses->setEncryptedSeed(encrypted);

	this->receivingAddresses->init(encoder);
}

void WalletAccount::initChangeAddressStore(const HdWalletSeed *rootAccountSeed,
		const IWalletDataEncoder *encoder, int numAddressInThisGroup) {
	this->changeAddresses = new ChangeAddressStore(this->zone, numAddressInThisGroup, this->accountBaseDir);

	HdWalletSeed* s = rootAccountSeed->indexedSeed(2); __STP(s);
	HdWalletSeed* encrypted = encoder->encode(s);

	this->changeAddresses->setEncryptedSeed(encrypted);
	this->changeAddresses->init(encoder);
}

AddressDescriptor* WalletAccount::getReceivingAddressDescriptor(int i) const noexcept {
	return this->receivingAddresses->getAddressDescriptor(i);
}

//AddressDescriptor* WalletAccount::getChangeAddressDescriptor(int i) const noexcept {
//	return this->changeAddresses->getAddressDescriptor(i);
//}

//const BalanceAddress* WalletAccount::getReceivingAddress(int i) const noexcept {
//	return this->receivingAddresses->getAddress(i);
//}

//const BalanceAddress* WalletAccount::getChangeAddress(int i) const noexcept {
//	return this->changeAddresses->getAddress(i);
//}

const BalanceAddress* WalletAccount::getAddress(const AddressDescriptor *desc) const noexcept {
	const BalanceAddress* ret = this->receivingAddresses->getAddress(desc);

	return ret != nullptr ? ret : this->changeAddresses->getAddress(desc);
}

IMuSigSigner* WalletAccount::getSigner(const AddressDescriptor *desc, const IWalletDataEncoder* encoder) {
	IMuSigSigner* signer = this->receivingAddresses->getSigner(desc, encoder);
	if(signer != nullptr){
		return signer;
	}

	signer = this->changeAddresses->getSigner(desc, encoder);
	return signer;
}

GenesisTransaction* WalletAccount::createGenesisTransaction(const BalanceUnit amount, int pos) {
	GenesisTransactionHandler handler(this);
	return handler.createTransaction(amount, pos);
}

BalanceTransferTransaction* WalletAccount::createBalanceTransferTransaction(
				const AddressDescriptor *dest, const BalanceUnit amount, const BalanceUnit feeRate, bool feeIncluded, const IWalletDataEncoder* encoder) {
	HdWalletAccountTrxBuilderContext context(this, encoder);

	return createBalanceTransferTransaction(dest, amount, feeRate, feeIncluded, encoder, &context);
}

BalanceTransferTransaction* WalletAccount::createBalanceTransferTransaction(
		const AddressDescriptor *dest, const BalanceUnit amount, const BalanceUnit feeRate, bool feeIncluded, const IWalletDataEncoder *encoder,
		ITransactionBuilderContext *context) {
	BalanceTransactionWalletHandler handler(this);

	return handler.createTransaction(dest, amount, feeRate, feeIncluded, encoder, context);
}

RegisterVotePoolTransaction* WalletAccount::createRegisterVotePoolTransaction(
		const NodeIdentifierSource *source, const BalanceUnit& feeRate, const AddressDescriptor* addressDesc, const IWalletDataEncoder *encoder) {
	HdWalletAccountTrxBuilderContext context(this, encoder);

	return createRegisterVotePoolTransaction(source, feeRate, addressDesc, encoder, &context);
}


RegisterVotePoolTransaction* WalletAccount::createRegisterVotePoolTransaction(
		const NodeIdentifierSource *source, const BalanceUnit &feeRate, const AddressDescriptor *addressDesc, const IWalletDataEncoder *encoder,
		ITransactionBuilderContext *context) {
	RegisterVotePoolTransactionWalletHandler handler(this);
	return handler.createTransaction(source, feeRate, addressDesc, encoder, context);
}

RegisterTicketTransaction* WalletAccount::createRegisterTicketTransaction(
		const NodeIdentifier *nodeId, const BalanceUnit& stakeAmount, const BalanceUnit& feeRate, const AddressDescriptor *addressDesc,
		const IWalletDataEncoder *encoder) {
	HdWalletAccountTrxBuilderContext context(this, encoder);

	return createRegisterTicketTransaction(nodeId, stakeAmount, feeRate, addressDesc, encoder, &context);
}

RegisterTicketTransaction* WalletAccount::createRegisterTicketTransaction(
		const NodeIdentifier *nodeId, const BalanceUnit &stakeAmount, const BalanceUnit &feeRate, const AddressDescriptor *addressDesc, const IWalletDataEncoder *encoder,
		ITransactionBuilderContext *context) {
	RegisterTicketTransactionWalletHandler handler(this);

	return handler.createTransaction(nodeId, stakeAmount, feeRate, addressDesc, encoder, context);
}

void WalletAccount::importTransaction(const AbstractBlockchainTransaction *trx) {
	uint8_t type = trx->getType();

	AbstractWalletTransactionHandler* handler = nullptr;

	switch(type){
	case AbstractBlockchainTransaction::TRX_TYPE_GENESIS:
		handler = new GenesisTransactionHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_BANANCE_TRANSFER:
		handler = new BalanceTransactionWalletHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_REGISTER_VOTE_POOL:
		handler = new RegisterVotePoolTransactionWalletHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_REGISTER_TICKET:
		handler = new RegisterTicketTransactionWalletHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_VOTE_BLOCK:
		handler = new VoteBlockTransactionWalletHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_REVOKE_MISSED_TICKET:
		handler = new RevokeMissedTicketWalletHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_REVOKE_MISS_VOTED_TICKET:
		handler = new RevokeMissVotedTicketWalletHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_COIN_BASE:
		handler = new CoinbaseTransactionWalletHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_STAKE_BASE:
		handler = new StakeBaseTransactionWalletHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_SMARTCONTRACT_NOP:
		handler = new SmartcontractNopTransactionWalletHandler(this);
		break;
	case AbstractBlockchainTransaction::TRX_TYPE_ICC_NOP:
		handler = new IccNopTransactionWalletHandler(this);
		break;
	default:
		break;
	}
	__STP(handler);

	ExceptionThrower<InvalidTransactionException>::throwExceptionIfCondition(handler == nullptr, L"invalid transaction type.", __FILE__, __LINE__);

	handler->importTransaction(trx);
}

AbstractBlockchainTransaction* WalletAccount::findTransaction(	const TransactionId *trxId) {
	return this->trxRepo->findTransaction(trxId);
}

void WalletAccount::removeTransaction(const TransactionId* trxId){
	this->trxRepo->removeTransaction(trxId);
}

bool WalletAccount::hasAddress(const AddressDescriptor *addressDesc) {
	return findAddressDiscriptor(this->receivingAddresses, addressDesc) >= 0
			|| findAddressDiscriptor(this->changeAddresses, addressDesc) >= 0;
}

int WalletAccount::findAddressDiscriptor(const AbstractAddressStore* store, const AddressDescriptor *addressDesc) {
	int ret = -1;

	int maxLoop = store->size();
	for(int i = 0; i != maxLoop; ++i){
		AddressDescriptor* desc = store->getAddressDescriptor(i); __STP(desc);
		if(desc->compareTo(addressDesc) == 0){
			ret = i;
			break;
		}
	}

	return ret;
}

BalanceUnit WalletAccount::getTotalAmount() const noexcept {
	return this->trxRepo->getTotalAmount();
}

void WalletAccount::save() {
	{
		ByteBuffer* buff = this->encryptedSeed->getByteBuffer();
		this->store->addBinaryValue(KEY_ENCRYPTED_SEED, buff->array(), buff->limit());
	}

	this->receivingAddresses->save();
	this->changeAddresses->save();

}

void WalletAccount::load(const IWalletDataEncoder *encoder) {
	this->store->load();
	{
		ByteBuffer* buff = this->store->getBinaryValue(KEY_ENCRYPTED_SEED); __STP(buff);
		delete this->encryptedSeed;
		this->encryptedSeed = new HdWalletSeed((const char*)buff->array(), buff->limit());
	}

	this->receivingAddresses = new ReceivingAddressStore(0, 512, this->accountBaseDir);
	this->changeAddresses = new ChangeAddressStore(0, 0, this->accountBaseDir);

	this->receivingAddresses->load(encoder);
	this->changeAddresses->load(encoder);

	this->trxRepo = new WalletAccountTrxRepository(this);
	this->trxRepo->open();
}

ArrayList<AbstractBlockchainTransaction>* WalletAccount::getTransactions() const {
	ArrayList<AbstractBlockchainTransaction>* list = new ArrayList<AbstractBlockchainTransaction>();

	BtreeScanner* scanner = this->trxRepo->getScanner(); __STP(scanner);
	while(scanner->hasNext()){
		const IBlockObject* obj = scanner->next();
		const TransactionData* data = dynamic_cast<const TransactionData*>(obj);

		list->addElement(dynamic_cast<AbstractBlockchainTransaction*>(data->getTrx()->copyData()));
	}

	return list;
}

const BloomFilter1024* WalletAccount::getBloomFilter(const IWalletDataEncoder* encoder) {
	if(this->bloomFilter == nullptr){
		createBloomFilter(encoder);
	}

	return this->bloomFilter;
}

void WalletAccount::createBloomFilter(const IWalletDataEncoder* encoder) {
	delete this->bloomFilter;

	this->bloomFilter = new BloomFilter1024();

	this->receivingAddresses->exportAddress2Filger(this->bloomFilter);
	this->changeAddresses->exportAddress2Filter(this->bloomFilter, encoder);
}

bool WalletAccount::checkAddress(const AddressDescriptor *desc) const {
	return this->receivingAddresses->hasAddress(desc) || this->changeAddresses->hasAddress(desc);
}

} /* namespace codablecash */
