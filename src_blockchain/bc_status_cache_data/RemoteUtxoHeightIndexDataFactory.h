/*
 * RemoteUtxoHeightIndexDataFactory.h
 *
 *  Created on: Jul 8, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_DATA_REMOTEUTXOHEIGHTINDEXDATAFACTORY_H_
#define BC_STATUS_CACHE_DATA_REMOTEUTXOHEIGHTINDEXDATAFACTORY_H_

#include "btree/AbstractBtreeDataFactory.h"

using namespace alinous;

namespace codablecash {

class RemoteUtxoHeightIndexDataFactory : public AbstractBtreeDataFactory {
public:
	RemoteUtxoHeightIndexDataFactory();
	virtual ~RemoteUtxoHeightIndexDataFactory();

	virtual IBlockObject* makeDataFromBinary(ByteBuffer* in);

	virtual void registerData(const AbstractBtreeKey* key, const IBlockObject* data, DataNode* dataNode, BtreeStorage* store) const;
	virtual bool beforeRemove(DataNode* dataNode, BtreeStorage* store, const AbstractBtreeKey* key) const;

	virtual AbstractBtreeDataFactory* copy() const noexcept;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_DATA_REMOTEUTXOHEIGHTINDEXDATAFACTORY_H_ */
