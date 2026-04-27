/*
 * StaticInstanceHolder.cpp
 *
 *  Created on: 2019/11/14
 *      Author: iizuka
 */

#include "instance/instance_ref_static/NumericConstHolder.h"
#include "instance/instance_ref_static/StaticConstInstanceHolder.h"
#include "instance/instance_ref_static/StringConstHolder.h"


namespace alinous {

StaticConstInstanceHolder::StaticConstInstanceHolder() { // @suppress("Class members should be properly initialized")
	this->numeric = new NumericConstHolder();
	this->stringConsts = new StringConstHolder();
}

StaticConstInstanceHolder::~StaticConstInstanceHolder() {
	delete this->numeric;
	delete this->stringConsts;
}

PrimitiveReference* StaticConstInstanceHolder::newNumericConstReferenece(int64_t value, uint8_t type, VirtualMachine* vm) {
	return this->numeric->newNumericConstReferenece(value, type, vm);
}

VmStringInstance* StaticConstInstanceHolder::newStringConstInstance(VmRootReference* rootRef, const UnicodeString* str, VirtualMachine* vm) {
	return this->stringConsts->newStringConstInstance(rootRef, str, vm);
}

void StaticConstInstanceHolder::removeStringConst(VmRootReference* rootRef, VirtualMachine* vm) noexcept {
	this->stringConsts->removeInnerReferences(rootRef, vm);
}

void StaticConstInstanceHolder::removeInnerReferences(VmRootReference* rootRef, VirtualMachine* vm) noexcept {
	this->numeric->removeInnerReferences(rootRef, vm);
}

} /* namespace alinous */
