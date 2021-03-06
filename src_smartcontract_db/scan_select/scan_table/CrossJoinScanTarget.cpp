/*
 * CrossJoinScanTarget.cpp
 *
 *  Created on: 2020/08/08
 *      Author: iizuka
 */

#include "scan_select/scan_table/CrossJoinScanTarget.h"

#include "base/UnicodeString.h"

namespace codablecash {

CrossJoinScanTarget::CrossJoinScanTarget() {

}

CrossJoinScanTarget::~CrossJoinScanTarget() {

}

const UnicodeString* CrossJoinScanTarget::toString() noexcept {
	if(this->str == nullptr){
		resetStr();
		this->str = new UnicodeString(L"");

		this->str->append(this->left->toString());

		this->str->append(L" CROSS JOIN ");

		this->str->append(this->right->toString());

		addConditionString();
	}

	return this->str;
}

AbstractScannerFactory* CrossJoinScanTarget::getScanFactory(VirtualMachine* vm, SelectScanPlanner* planner) {
	// FIXME CrossJoinScanTarget::getScanFactory
	return nullptr;
}


} /* namespace codablecash */
