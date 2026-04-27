/*
 * InstanceIdIndexKeyFactory.cpp
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor_index/InstanceIdIndexKeyFactory.h"
#include "smartcontract_executor_index/InstanceIdIndexKey.h"


namespace codablecash {

InstanceIdIndexKeyFactory::InstanceIdIndexKeyFactory() {

}

InstanceIdIndexKeyFactory::~InstanceIdIndexKeyFactory() {

}

AbstractBtreeKey* InstanceIdIndexKeyFactory::fromBinary(uint32_t keyType, ByteBuffer *in) const {
	if(keyType == INSTANCE_INDEX_KEY){
		return InstanceIdIndexKey::fromBinary(in);
	}

	return BtreeKeyFactory::fromBinary(keyType, in);
}

BtreeKeyFactory* InstanceIdIndexKeyFactory::copy() const noexcept {
	return new InstanceIdIndexKeyFactory();
}

} /* namespace codablecash */
