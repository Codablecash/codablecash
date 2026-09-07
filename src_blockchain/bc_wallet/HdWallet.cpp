/*
 * HdWallet.cpp
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#include "bc_wallet/HdWallet.h"
#include "bc_wallet/WalletAccount.h"
#include "bc_wallet/HdWalletSeed.h"

#include "bc_wallet_encoder/IWalletDataEncoder.h"

#include "base_io/File.h"
#include "base_io/ByteBuffer.h"

#include "bc_base_conf_store/StatusStore.h"

#include "base/StackRelease.h"
#include "bc_wallet_filter/BloomFilter1024.h"

namespace codablecash {

HdWallet::HdWallet(const File* baseDir) {
	this->baseDir = new File(*baseDir);
	this->accounts = new ArrayList<WalletAccount>();
	this->encodedSeed = nullptr;

	this->store = new StatusStore(baseDir, HdWallet::STORE_NAME);

	this->defaultZone = 0;
}

HdWallet::~HdWallet() {
	close();

	delete this->baseDir;
}

HdWallet* HdWallet::create(const File *dir, const HdWalletSeed *seed, uint16_t defaultZone, const IWalletDataEncoder* encoder, int defaultMaxAddress) {
	HdWallet* wallet = new HdWallet(dir);
	wallet->setSeed(seed, encoder);
	wallet->setDefaultZone(defaultZone);

	wallet->newAccount(seed, defaultZone, encoder, defaultMaxAddress);
	wallet->save();

	return wallet;
}

HdWallet* HdWallet::loadWallet(const File *dir, const IWalletDataEncoder *encoder) {
	HdWallet* wallet = new HdWallet(dir);
	wallet->load(encoder);

	return wallet;
}

void HdWallet::setSeed(const HdWalletSeed *seed, const IWalletDataEncoder* encoder) {
	delete this->encodedSeed;

	this->encodedSeed = seed->encodedSeed(encoder);
}

void HdWallet::newAccount(const HdWalletSeed *rootSeed, uint16_t zone, const IWalletDataEncoder* encoder, int maxAddress) {
	int accountIndex = this->accounts->size();
	WalletAccount* account = WalletAccount::newAccount(this->baseDir, rootSeed, accountIndex, zone, encoder, maxAddress);

	this->accounts->addElement(account);
	save();
}

void HdWallet::save() {
	{
		ByteBuffer* buff = this->encodedSeed->getByteBuffer();
		this->store->addBinaryValue(KEY_ENCODED_SEED, buff->array(), buff->limit());
	}
	this->store->addShortValue(KEY_NUM_ACCOUNTS, this->accounts->size());
	this->store->addShortValue(KEY_DEFAULT_ZONE, this->defaultZone);
}

void HdWallet::load(const IWalletDataEncoder *encoder) {
	this->store->load();
	{
		ByteBuffer* buff = this->store->getBinaryValue(KEY_ENCODED_SEED); __STP(buff);
		this->encodedSeed = new HdWalletSeed((const char*)buff->array(), buff->limit());
	}

	int numAccounts = this->store->getShortValue(KEY_NUM_ACCOUNTS);
	for(int i = 0; i != numAccounts; ++i){
		WalletAccount* account = WalletAccount::loadAccount(this->baseDir, i, encoder);
		this->accounts->addElement(account);
	}

	this->defaultZone = this->store->getShortValue(KEY_DEFAULT_ZONE);
}

void HdWallet::close() {
	if(this->store != nullptr){
		delete this->store, this->store = nullptr;
	}

	if(this->accounts != nullptr){
		this->accounts->deleteElements();
		delete this->accounts, this->accounts = nullptr;
	}

	if(this->encodedSeed != nullptr){
		delete this->encodedSeed, this->encodedSeed = nullptr;
	}
}

HdWalletSeed* HdWallet::getRootSeed(const IWalletDataEncoder* encoder) const {
	HdWalletSeed* rootSeed = encoder->decode(this->encodedSeed);

	return rootSeed;
}

ArrayList<BloomFilter1024>* HdWallet::getBloomFilters(const IWalletDataEncoder* encoder) const noexcept {
	ArrayList<BloomFilter1024>* list = new ArrayList<BloomFilter1024>();

	int maxLoop = this->accounts->size();
	for(int i = 0; i != maxLoop; ++i){
		WalletAccount* account = this->accounts->get(i);
		const BloomFilter1024* f = account->getBloomFilter(encoder);

		list->addElement(dynamic_cast<BloomFilter1024*>(f->copyData()));
	}

	return list;
}

WalletAccount* HdWallet::getZoneAccount(uint16_t zone) const noexcept {
	WalletAccount* retaccount = nullptr;

	int maxLoop = this->accounts->size();
	for(int i = 0; i != maxLoop; ++i){
		WalletAccount* account = this->accounts->get(i);
		uint16_t z = account->getZone();

		if(z == zone){
			retaccount = account;
		}
	}

	return retaccount;
}

} /* namespace codablecash */
