/*
 * RemoteUtxoHeightIndexDataFactory.cpp
 *
 *  Created on: Jul 8, 2026
 *      Author: iizuka
 */

#include "bc_status_cache_data/RemoteUtxoHeightIndexDataFactory.h"
#include "bc_status_cache_data/RemoteUtxoHeightIndexData.h"

#include "btree/DataNode.h"
#include "btree/BtreeStorage.h"

#include "base/StackRelease.h"

#include "filestore_block/IBlockObject.h"

namespace codablecash {

RemoteUtxoHeightIndexDataFactory::RemoteUtxoHeightIndexDataFactory() {

}

RemoteUtxoHeightIndexDataFactory::~RemoteUtxoHeightIndexDataFactory() {

}

IBlockObject* RemoteUtxoHeightIndexDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return RemoteUtxoHeightIndexData::createFromBinary(in);
}

void RemoteUtxoHeightIndexDataFactory::registerData(const AbstractBtreeKey *key, const IBlockObject *data, DataNode *dataNode, BtreeStorage *store) const {
	uint64_t dataFpos = dataNode->getDataFpos();
	if(dataFpos != 0){
		IBlockObject* obj = store->loadData(dataFpos); __STP(obj);

		RemoteUtxoHeightIndexData* baseValue = dynamic_cast<RemoteUtxoHeightIndexData*>(obj);
		const RemoteUtxoHeightIndexData* newValue = dynamic_cast<const RemoteUtxoHeightIndexData*>(data);

		baseValue->join(newValue);
		dataFpos = store->storeData(baseValue, dataFpos);
		dataNode->setDataFpos(dataFpos);

		return;
	}

	dataFpos = store->storeData(data);
	dataNode->setDataFpos(dataFpos);
}

bool RemoteUtxoHeightIndexDataFactory::beforeRemove(DataNode *dataNode, BtreeStorage *store, const AbstractBtreeKey *key) const {
	return true; // remove
}

AbstractBtreeDataFactory* RemoteUtxoHeightIndexDataFactory::copy() const noexcept {
	return new RemoteUtxoHeightIndexDataFactory();
}

} /* namespace codablecash */
