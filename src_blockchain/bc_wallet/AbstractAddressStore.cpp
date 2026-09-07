/*
 * AbstractAddressStore.cpp
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#include "bc_wallet/AbstractAddressStore.h"
#include "bc_wallet/HdWalletSeed.h"
#include "bc_wallet/AddressAndPrivateKey.h"

#include "bc_wallet_encoder/IWalletDataEncoder.h"

#include "bc_wallet_trx_repo/WalletAccountTrxRepository.h"

#include "bc_base/BalanceAddress.h"
#include "bc_base/AddressDescriptor.h"

#include "ecda/ScPrivateKey.h"
#include "ecda/ScPublicKey.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "musig/SimpleMuSigSigner.h"

#include "bc_base_conf_store/StatusStore.h"
#include "bc_base_conf_store/ShortValueConfigStoreValue.h"

#include "bc_base/BalanceUnit.h"

#include "bc_trx_balance/BalanceUtxo.h"

namespace codablecash {

AbstractAddressStore::AbstractAddressStore(uint16_t zone, const File* baseDir, const wchar_t* storeName) {
	this->encryptedSeed = nullptr;
	this->addressSerial = 0;
	this->zone = zone;

	this->list = new ArrayList<AddressAndPrivateKey>();

	UnicodeString name(storeName);
	this->store = new StatusStore(baseDir, &name);
}

AbstractAddressStore::~AbstractAddressStore() {
	delete this->encryptedSeed;

	this->list->deleteElements();
	delete this->list;

	delete this->store;
}

void AbstractAddressStore::setEncryptedSeed(HdWalletSeed *encrypted) noexcept {
	delete this->encryptedSeed;
	this->encryptedSeed = encrypted;
}

AddressDescriptor* AbstractAddressStore::getAddressDescriptor(int i) const noexcept {
	const BalanceAddress* address = getAddress(i);

	return address->toAddressDescriptor();
}

const BalanceAddress* AbstractAddressStore::getAddress(int i) const noexcept {
	AddressAndPrivateKey* pkey = this->list->get(i);
	const BalanceAddress* address = pkey->getAddress();

	return address;
}

const BalanceAddress* AbstractAddressStore::getAddress(const AddressDescriptor *dest) const noexcept {
	const BalanceAddress* ret = nullptr;

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		const BalanceAddress* addr = getAddress(i);
		AddressDescriptor* d = addr->toAddressDescriptor(); __STP(d);
		if(d->compareTo(dest) == 0){
			ret = addr;
			break;
		}
	}

	return ret;
}

bool AbstractAddressStore::hasAddress(const AddressDescriptor *dest) const noexcept {
	return getAddress(dest) != nullptr;
}

AddressAndPrivateKey* AbstractAddressStore::createNewAddressAndPrivateKey(const IWalletDataEncoder *encoder, uint64_t serial) {
	HdWalletSeed* seed = encoder->decode(this->encryptedSeed); __STP(seed);

	BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
	ScPrivateKey* privateKey = new ScPrivateKey(bigInt, serial + 1);

	ScPublicKey publicKey = privateKey->generatePublicKey();

	AddressAndPrivateKey* ret = new AddressAndPrivateKey();
	ret->setPrivateKey(privateKey);

	BalanceAddress* address = BalanceAddress::createAddress(this->zone, &publicKey);
	ret->setBalanceAddress(address);

	return ret;
}

IMuSigSigner* AbstractAddressStore::getSigner(const AddressDescriptor *desc, const IWalletDataEncoder* encoder) {
	IMuSigSigner* ret = nullptr;

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		AddressAndPrivateKey* key = this->list->get(i);

		const BalanceAddress* address = key->getAddress();
		AddressDescriptor* descriptor = address->toAddressDescriptor(); __STP(descriptor);
		if(desc->compareTo(descriptor) == 0){
			const ScPrivateKey* pkey = key->getPrivateKey();
			ret = new SimpleMuSigSigner(pkey->getKeyvalue());
			break;
		}
	}

	return ret;
}


void AbstractAddressStore::__save() {
	this->store->addShortValue(KEY_ZONE, this->zone);

	{
		ByteBuffer* buff = this->encryptedSeed->getByteBuffer();
		this->store->addBinaryValue(KEY_ENCRYPTED_SEED, buff->array(), buff->limit());
	}

	this->store->addLongValue(KEY_ADDRESS_SERIAL, this->addressSerial);
}

void AbstractAddressStore::__load() {
	this->zone = this->store->getShortValue(KEY_ZONE);

	{
		ByteBuffer* buff = this->store->getBinaryValue(KEY_ENCRYPTED_SEED); __STP(buff);
		delete this->encryptedSeed;
		this->encryptedSeed = new HdWalletSeed((const char*)buff->array(), buff->limit());
	}

	this->addressSerial = this->store->getLongValue(KEY_ADDRESS_SERIAL);
}

} /* namespace codablecash */
