/*
 * RemoteUtxoDataFactory.cpp
 *
 *  Created on: Jul 8, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_data/RemoteUtxoDataFactory.h"
#include "bc_status_cache_data/RemoteUtxoData.h"

#include "btree/BtreeStorage.h"
#include "btree/DataNode.h"


namespace codablecash {

RemoteUtxoDataFactory::RemoteUtxoDataFactory() {

}

RemoteUtxoDataFactory::~RemoteUtxoDataFactory() {

}

IBlockObject* RemoteUtxoDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return RemoteUtxoData::createFromBinary(in);
}

void RemoteUtxoDataFactory::registerData(const AbstractBtreeKey *key, const IBlockObject *data, DataNode *dataNode, BtreeStorage *store) const {
	AbstractBtreeDataFactory::registerData(key, data, dataNode, store);
}

bool RemoteUtxoDataFactory::beforeRemove(DataNode *dataNode, BtreeStorage *store, const AbstractBtreeKey *key) const {
	return true;
}

AbstractBtreeDataFactory* RemoteUtxoDataFactory::copy() const noexcept {
	return new RemoteUtxoDataFactory();
}

} /* namespace codablecash */
