/*
 * ChangeAddressStore.cpp
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#include "bc_wallet/ChangeAddressStore.h"

#include "bc_base_conf_store/StatusStore.h"

#include "base/StackRelease.h"

#include "bc_base/AddressDescriptor.h"
#include "bc_wallet_filter/BloomFilter1024.h"

namespace codablecash {

ChangeAddressStore::ChangeAddressStore(uint16_t zone, int numAddressInThisGroup, const File* baseDir)
		: AbstractAddressStore(zone, baseDir, ChangeAddressStore::STORE_NAME) {
	this->currentIndex = 0;
	this->numAddressInThisGroup = numAddressInThisGroup;
}

ChangeAddressStore::~ChangeAddressStore() {

}

void ChangeAddressStore::init(const IWalletDataEncoder *encoder) {
	save();
}

AddressDescriptor* ChangeAddressStore::getNextChangeAddress(const IWalletDataEncoder *encoder) noexcept {
	if(this->currentIndex >= this->numAddressInThisGroup){
		this->currentIndex = 0;
	}

	if(this->currentIndex >= this->addressSerial && this->list->size() < this->numAddressInThisGroup){
		AddressAndPrivateKey* pair = createNewAddressAndPrivateKey(encoder, this->addressSerial++);
		this->list->addElement(pair);
		save();
	}
	return getAddressDescriptor(this->currentIndex++);
}

void ChangeAddressStore::save() {
	__save();

	this->store->addLongValue(KEY_CURRENT_INDEX, this->currentIndex);
	this->store->addLongValue(KEY_NUM_GROUP, this->numAddressInThisGroup);
}

void ChangeAddressStore::load(const IWalletDataEncoder *encoder) {
	this->store->load();
	__load();

	this->currentIndex = this->store->getLongValue(KEY_CURRENT_INDEX);
	this->numAddressInThisGroup = this->store->getLongValue(KEY_NUM_GROUP);

	for(int i = 0; i != this->addressSerial; ++i){
		AddressAndPrivateKey* pair = createNewAddressAndPrivateKey(encoder, i);

		this->list->addElement(pair);
	}
}

void ChangeAddressStore::exportAddress2Filter(BloomFilter1024 *filter, const IWalletDataEncoder *encoder) {
	int maxLoop = this->numAddressInThisGroup;
	for(int i = 0; i != maxLoop; ++i){
		AddressDescriptor* desc = getNextChangeAddress(encoder); __STP(desc);

		filter->add(desc);
	}
}

} /* namespace codablecash */
