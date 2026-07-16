/*
 * VotingBlockStatusDataFactory.cpp
 *
 *  Created on: 2023/05/01
 *      Author: iizuka
 */

#include "bc_status_cache_context_finalizer/VotingBlockStatusDataFactory.h"
#include "bc_status_cache_context_finalizer/VotingBlockStatus.h"

#include "btree/DataNode.h"
#include "btree/BtreeStorage.h"

namespace codablecash {

VotingBlockStatusDataFactory::VotingBlockStatusDataFactory() {

}

VotingBlockStatusDataFactory::~VotingBlockStatusDataFactory() {

}

IBlockObject* VotingBlockStatusDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return VotingBlockStatus::createFromBinary(in);
}

void VotingBlockStatusDataFactory::registerData(const AbstractBtreeKey *key,
		const IBlockObject *data, DataNode *dataNode, BtreeStorage *store) const {
	AbstractBtreeDataFactory::registerData(key, data, dataNode, store);
}
/*
bool VotingBlockStatusDataFactory::beforeRemove(DataNode *dataNode,
		BtreeStorage *store, const AbstractBtreeKey *key) const {
	return true; // always remove
}*/

AbstractBtreeDataFactory* VotingBlockStatusDataFactory::copy() const noexcept {
	return new VotingBlockStatusDataFactory();
}

} /* namespace codablecash */
