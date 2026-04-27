/*
 * VmNullprtInstance.cpp
 *
 *  Created on: Jan 8, 2026
 *      Author: iizuka
 */

#include "instance/VmNullprtInstance.h"
#include "instance/VmInstanceTypesConst.h"

#include "instance/instance_array/ArrayReference.h"

#include "instance/instance_ref/ObjectReference.h"

#include "instance/instance_dom/DomRuntimeReference.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "ext_binary/ExtNullPtrObject.h"

#include "vm/VirtualMachine.h"

#include "base/UnicodeString.h"

namespace alinous {

const UnicodeString VmNullprtInstance::NULL_STR(L"null");

VmNullprtInstance::VmNullprtInstance(uint64_t serial, uint8_t atype) : AbstractVmInstance(VmInstanceTypesConst::INST_NULL, serial) {
	this->atype = atype;
}

VmNullprtInstance::~VmNullprtInstance() {

}

AbstractReference* VmNullprtInstance::wrap(IAbstractVmInstanceSubstance *owner, VirtualMachine *vm) {
	AbstractReference* ref = nullptr;
	if(this->atype == AnalyzedType::TYPE_OBJECT){
		ref = new(vm) ObjectReference(owner, VmInstanceTypesConst::REF_OBJ, ObjectReference::CLASS_INSTANCE, vm->publishInstanceSerial());
	}
	else if(this->atype == AnalyzedType::TYPE_STRING){
		ref = new(vm) ObjectReference(owner, VmInstanceTypesConst::REF_OBJ, ObjectReference::STRING_INSTANCE, vm->publishInstanceSerial());
	}
	else if(this->atype == AnalyzedType::TYPE_OBJECT_ARRAY){
		ref = new(vm) ArrayReference(owner, vm, nullptr);
	}
	else if(this->atype == AnalyzedType::TYPE_DOM_VALUE ||
			this->atype == AnalyzedType::TYPE_DOM_ARRAY ||
			this->atype == AnalyzedType::TYPE_DOM_VALUE_PAIR ||
			this->atype == AnalyzedType::TYPE_DOM){
		ref = new(vm) DomRuntimeReference(owner, vm);
	}

	return ref;
}

uint8_t VmNullprtInstance::getInstType() const noexcept {
	return getType();
}

AnalyzedType VmNullprtInstance::getRuntimeType() const noexcept {
	return AnalyzedType(AnalyzedType::TYPE_NULL);
}

const VMemList<AbstractReference>* VmNullprtInstance::getInstReferences() const noexcept {
	return nullptr;
}

int VmNullprtInstance::instHashCode() const noexcept {
	return 0;
}

bool VmNullprtInstance::instIsPrimitive() const noexcept {
	return false;
}

bool VmNullprtInstance::instIsNull() const noexcept {
	return true;
}

int VmNullprtInstance::instValueCompare(const IAbstractVmInstanceSubstance *right) const noexcept {
	return right->instIsNull() ? 0 : -1;
}

AbstractExtObject* VmNullprtInstance::instToClassExtObject(const UnicodeString *name, VTableRegistory *table) {
	return new ExtNullPtrObject(name, this->atype);
}

IAbstractVmInstanceSubstance* VmNullprtInstance::getInstance() noexcept {
	return this;
}

int VmNullprtInstance::valueCompare(const IAbstractVmInstanceSubstance *right) const noexcept {
	return instValueCompare(right);
}

const UnicodeString* VmNullprtInstance::toString() const noexcept {
	return &VmNullprtInstance::NULL_STR;
}

} /* namespace alinous */
