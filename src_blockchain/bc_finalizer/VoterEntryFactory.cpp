/*
 * VoterEntryFactory.cpp
 *
 *  Created on: 2023/05/09
 *      Author: iizuka
 */

#include "bc_finalizer/VoterEntryFactory.h"

#include "btree/BtreeStorage.h"
#include "btree/DataNode.h"

#include "bc_finalizer/VoterEntry.h"

namespace codablecash {

VoterEntryFactory::VoterEntryFactory() {

}

VoterEntryFactory::~VoterEntryFactory() {

}

IBlockObject* VoterEntryFactory::makeDataFromBinary(ByteBuffer *in) {
	return VoterEntry::createFromBinary(in);
}

void VoterEntryFactory::registerData(const AbstractBtreeKey *key,
		const IBlockObject *data, DataNode *dataNode, BtreeStorage *store) const {
	AbstractBtreeDataFactory::registerData(key, data, dataNode, store);
}

AbstractBtreeDataFactory* VoterEntryFactory::copy() const noexcept {
	return new VoterEntryFactory();
}

} /* namespace codablecash */
