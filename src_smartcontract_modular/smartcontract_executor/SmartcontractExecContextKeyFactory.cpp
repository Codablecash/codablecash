/*
 * SmartcontractExecContextKeyFactory.cpp
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor/SmartcontractExecContextKeyFactory.h"
#include "smartcontract_executor/SmartcontractExecContextKey.h"


namespace codablecash {

SmartcontractExecContextKeyFactory::SmartcontractExecContextKeyFactory() {

}

SmartcontractExecContextKeyFactory::~SmartcontractExecContextKeyFactory() {

}

AbstractBtreeKey* SmartcontractExecContextKeyFactory::fromBinary(uint32_t keyType, ByteBuffer *in) const {
	if(keyType == SMARTCONTRACT_EXEC_ID_KEY){
		return SmartcontractExecContextKey::fromBinary(in);
	}

	return BtreeKeyFactory::fromBinary(keyType, in);
}

BtreeKeyFactory* SmartcontractExecContextKeyFactory::copy() const noexcept {
	return new SmartcontractExecContextKeyFactory();
}

} /* namespace codablecash */
