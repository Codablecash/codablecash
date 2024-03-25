/*
 * VotingBlockStatusDataFactory.h
 *
 *  Created on: 2023/05/01
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_CONTEXT_FINALIZER_VOTINGBLOCKSTATUSDATAFACTORY_H_
#define BC_STATUS_CACHE_CONTEXT_FINALIZER_VOTINGBLOCKSTATUSDATAFACTORY_H_

#include "btree/AbstractBtreeDataFactory.h"

using namespace alinous;

namespace codablecash {

class VotingBlockStatusDataFactory : public AbstractBtreeDataFactory {
public:
	VotingBlockStatusDataFactory();
	virtual ~VotingBlockStatusDataFactory();

	virtual IBlockObject* makeDataFromBinary(ByteBuffer* in);

	virtual void registerData(const AbstractBtreeKey* key, const IBlockObject* data, DataNode* dataNode, BtreeStorage* store) const;
	//virtual bool beforeRemove(DataNode* dataNode, BtreeStorage* store, const AbstractBtreeKey* key) const;

	virtual AbstractBtreeDataFactory* copy() const noexcept;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_CONTEXT_FINALIZER_VOTINGBLOCKSTATUSDATAFACTORY_H_ */
