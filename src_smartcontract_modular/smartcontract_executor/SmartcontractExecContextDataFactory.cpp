/*
 * SmartcontractExecContextDataFactory.cpp
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor/SmartcontractExecContextDataFactory.h"
#include "smartcontract_executor/SmartcontractExecContextData.h"

#include "base_io/ByteBuffer.h"


namespace codablecash {

SmartcontractExecContextDataFactory::SmartcontractExecContextDataFactory() {

}

SmartcontractExecContextDataFactory::~SmartcontractExecContextDataFactory() {

}

IBlockObject* SmartcontractExecContextDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return SmartcontractExecContextData::createFromBinary(in);
}

AbstractBtreeDataFactory* SmartcontractExecContextDataFactory::copy() const noexcept {
	return new SmartcontractExecContextDataFactory();
}

} /* namespace codablecash */
