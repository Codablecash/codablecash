/*
 * SmartcontractExecResult.cpp
 *
 *  Created on: Dec 2, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor/SmartcontractExecResult.h"

#include "ext_binary/AbstractExtObject.h"


namespace codablecash {

SmartcontractExecResult::SmartcontractExecResult() {
	this->returnedValue = nullptr;
}

SmartcontractExecResult::~SmartcontractExecResult() {
	delete this->returnedValue;
}

void SmartcontractExecResult::setReturnedValue(AbstractExtObject *value) noexcept {
	this->returnedValue = value;
}

} /* namespace codablecash */
