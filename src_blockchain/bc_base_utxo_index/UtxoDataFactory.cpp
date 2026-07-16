/*
 * UtxoDataFactory.cpp
 *
 *  Created on: 2023/05/09
 *      Author: iizuka
 */

#include "bc_base_utxo_index/UtxoDataFactory.h"
#include "bc_base_utxo_index/UtxoData.h"

#include "btree/BtreeStorage.h"
#include "btree/DataNode.h"


namespace codablecash {

UtxoDataFactory::UtxoDataFactory() {

}

UtxoDataFactory::~UtxoDataFactory() {

}

IBlockObject* UtxoDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return UtxoData::fromBinary(in);
}

void UtxoDataFactory::registerData(const AbstractBtreeKey *key,
		const IBlockObject *data, DataNode *dataNode, BtreeStorage *store) const {
	AbstractBtreeDataFactory::registerData(key, data, dataNode, store);
}

bool UtxoDataFactory::beforeRemove(DataNode *dataNode, BtreeStorage *store, const AbstractBtreeKey *key) const {
	return true;
}

AbstractBtreeDataFactory* UtxoDataFactory::copy() const noexcept {
	return new UtxoDataFactory();
}

} /* namespace codablecash */
