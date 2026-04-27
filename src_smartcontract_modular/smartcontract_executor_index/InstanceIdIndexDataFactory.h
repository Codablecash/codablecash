/*
 * InstanceIdIndexDataFactory.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXDATAFACTORY_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXDATAFACTORY_H_

#include "btree/AbstractBtreeDataFactory.h"

using namespace alinous;

namespace codablecash {

class InstanceIdIndexDataFactory : public AbstractBtreeDataFactory {
public:
	InstanceIdIndexDataFactory();
	virtual ~InstanceIdIndexDataFactory();

	virtual IBlockObject* makeDataFromBinary(ByteBuffer* in);
	virtual AbstractBtreeDataFactory* copy() const noexcept;

	virtual void registerData(const AbstractBtreeKey* key, const IBlockObject* data, DataNode* dataNode, BtreeStorage* store) const;
	virtual bool beforeRemove(DataNode* dataNode, BtreeStorage* store, const AbstractBtreeKey* key) const;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_INSTANCEIDINDEXDATAFACTORY_H_ */
