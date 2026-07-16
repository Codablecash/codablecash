/*
 * TransactionIdKeyFactory.cpp
 *
 *  Created on: 2023/02/23
 *      Author: iizuka
 */

#include "bc_base_trx_index/TransactionDataFactory.h"
#include "bc_base_trx_index/TransactionData.h"

#include "btree/BtreeStorage.h"

#include "btree/DataNode.h"

#include "base/StackRelease.h"
namespace codablecash {

TransactionDataFactory::TransactionDataFactory() {

}

TransactionDataFactory::~TransactionDataFactory() {

}

IBlockObject* TransactionDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return TransactionData::fromBinary(in);
}

void TransactionDataFactory::registerData(const AbstractBtreeKey *key, const IBlockObject *data, DataNode *dataNode, BtreeStorage *store) const {
	AbstractBtreeDataFactory::registerData(key, data, dataNode, store);
}

bool TransactionDataFactory::beforeRemove(DataNode *dataNode,
		BtreeStorage *store, const AbstractBtreeKey *key) const {
	return true; // always remove
}

AbstractBtreeDataFactory* TransactionDataFactory::copy() const noexcept {
	return new TransactionDataFactory();
}

} /* namespace codablecash */
