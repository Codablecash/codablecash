/*
 * ExtNullPtrObject.cpp
 *
 *  Created on: 2020/03/28
 *      Author: iizuka
 */

#include "ext_binary/ExtNullPtrObject.h"

#include "instance/VmInstanceTypesConst.h"
#include "instance/VmNullprtInstance.h"

#include "base/UnicodeString.h"

#include "vm/VirtualMachine.h"

namespace alinous {

ExtNullPtrObject::ExtNullPtrObject(const UnicodeString* name, uint8_t atype) : AbstractExtObject(name, VmInstanceTypesConst::INST_NULL){
	this->atype = atype;
}

ExtNullPtrObject::~ExtNullPtrObject() {
}

bool ExtNullPtrObject::isNull() const noexcept {
	return true;
}

AbstractExtObject* ExtNullPtrObject::copy() const noexcept {
	return new ExtNullPtrObject(this->name, this->atype);
}

const UnicodeString* ExtNullPtrObject::toString() const noexcept {
	delete this->str;
	this->str = new UnicodeString(L"null");

	return this->str;
}

AbstractVmInstance* ExtNullPtrObject::toVmInstance(VirtualMachine *vm) {
	return new(vm) VmNullprtInstance(vm->publishInstanceSerial(), this->atype);
}

} /* namespace alinous */
