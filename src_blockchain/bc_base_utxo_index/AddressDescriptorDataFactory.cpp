/*
 * AddressDescriptorDataFactory.cpp
 *
 *  Created on: 2023/02/24
 *      Author: iizuka
 */

#include "bc_base_utxo_index/AddressDescriptorDataFactory.h"
#include "bc_base_utxo_index/AddressDescriptorData.h"

#include "btree/BtreeStorage.h"
#include "btree/DataNode.h"


namespace codablecash {

AddressDescriptorDataFactory::AddressDescriptorDataFactory() {

}

AddressDescriptorDataFactory::~AddressDescriptorDataFactory() {

}

IBlockObject* AddressDescriptorDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return AddressDescriptorData::fromBinary(in);
}

void AddressDescriptorDataFactory::registerData(const AbstractBtreeKey *key,
		const IBlockObject *data, DataNode *dataNode, BtreeStorage *store) const {
	AbstractBtreeDataFactory::registerData(key, data, dataNode, store);
}

bool AddressDescriptorDataFactory::beforeRemove(DataNode *dataNode,
		BtreeStorage *store, const AbstractBtreeKey *key) const {
	return true;
}

AbstractBtreeDataFactory* AddressDescriptorDataFactory::copy() const noexcept {
	return new AddressDescriptorDataFactory();
}

} /* namespace codablecash */
