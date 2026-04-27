/*
 * InstanceIdIndexDataFactory.cpp
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor_index/InstanceIdIndexDataFactory.h"
#include "smartcontract_executor_index/InstanceIdIndexData.h"
#include "smartcontract_executor_index/InstanceSessionContext.h"
#include "smartcontract_executor_index/InstanceIdIndexKey.h"

#include "btree/DataNode.h"
#include "btree/BtreeStorage.h"

#include "base/StackRelease.h"


namespace codablecash {

InstanceIdIndexDataFactory::InstanceIdIndexDataFactory() {


}

InstanceIdIndexDataFactory::~InstanceIdIndexDataFactory() {

}

IBlockObject* InstanceIdIndexDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return InstanceIdIndexData::createFromBinary(in);
}

AbstractBtreeDataFactory* InstanceIdIndexDataFactory::copy() const noexcept {
	return new InstanceIdIndexDataFactory();
}

void InstanceIdIndexDataFactory::registerData(const AbstractBtreeKey *key, const IBlockObject *data, DataNode *dataNode, BtreeStorage *store) const {
	uint64_t dataFpos = dataNode->getDataFpos();
	if(dataFpos != 0){
		IBlockObject* obj = store->loadData(dataFpos); __STP(obj);

		InstanceIdIndexData* baseValue = dynamic_cast<InstanceIdIndexData*>(obj);
		const InstanceIdIndexData* newValue = dynamic_cast<const InstanceIdIndexData*>(data);

		const InstanceSessionContext* ctx = newValue->get(0);
		InstanceSessionContext* cpy = dynamic_cast<InstanceSessionContext*>(ctx->copyData());

		baseValue->addSessionContext(cpy);
		baseValue->sort();

		dataFpos = store->storeData(baseValue, dataFpos);
		dataNode->setDataFpos(dataFpos);

		return;
	}

	dataFpos = store->storeData(data);
	dataNode->setDataFpos(dataFpos);
}

bool InstanceIdIndexDataFactory::beforeRemove(DataNode *dataNode, BtreeStorage *store, const AbstractBtreeKey *key) const {
	uint64_t dataFpos = dataNode->getDataFpos();

	IBlockObject* obj = store->loadData(dataFpos); __STP(obj);
	InstanceIdIndexData* baseValue = dynamic_cast<InstanceIdIndexData*>(obj);

	const InstanceIdIndexKey* fkey = dynamic_cast<const InstanceIdIndexKey*>(key);
	const CdbDatabaseSessionId* sessionId = fkey->getTrxId();

	baseValue->removeSessionId(sessionId);

	if(baseValue->isEmpty()){
		return true; // remove
	}

	dataFpos = store->storeData(baseValue, dataFpos);
	dataNode->setDataFpos(dataFpos);

	return false; // do not remove
}

} /* namespace codablecash */
