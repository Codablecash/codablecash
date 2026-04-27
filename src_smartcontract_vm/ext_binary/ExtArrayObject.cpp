/*
 * ExtArrayObject.cpp
 *
 *  Created on: 2020/02/18
 *      Author: iizuka
 */

#include "ext_binary/ExtArrayObject.h"

#include "instance/VmInstanceTypesConst.h"

#include "instance/instance_array/VmArrayInstance.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/PackageSpace.h"

#include "engine/sc/SmartContract.h"

#include "bc/ExceptionThrower.h"

#include "vm/exceptions.h"


using namespace codablecash;

namespace alinous {

ExtArrayObject::ExtArrayObject(const UnicodeString* name, int length, uint8_t atype, int dim, const UnicodeString* fqn) : AbstractExtObject(name, VmInstanceTypesConst::INST_ARRAY) {
	this->length = length;
	this->atype = atype;
	this->dim = dim;
	this->fqn = fqn != nullptr ? new UnicodeString(fqn) : nullptr;
}

ExtArrayObject::~ExtArrayObject() {
	this->array.deleteElements();
	delete this->fqn;
}

void ExtArrayObject::addInnerElement(AbstractExtObject* obj) noexcept {
	this->array.addElement(obj);
}

int ExtArrayObject::getLength() const noexcept {
	return this->length;
}

AbstractExtObject* ExtArrayObject::get(int i) const noexcept {
	return this->array.get(i);
}

AbstractExtObject* ExtArrayObject::copy() const noexcept {
	ExtArrayObject* newObj = new ExtArrayObject(getName(), this->length, this->atype, this->dim, this->fqn);

	int maxLoop = this->array.size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExtObject* obj = this->array.get(i);

		newObj->addInnerElement(obj == nullptr ? nullptr : obj->copy());
	}

	return newObj;
}

const UnicodeString* ExtArrayObject::toString() const noexcept {
	delete this->str;
	this->str = new UnicodeString(L"[");


	int maxLoop = this->array.size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExtObject* obj = this->array.get(i);

		if(i > 0){
			this->str->append(L", ");
		}

		this->str->append(obj->toString());
	}

	this->str->append(L"]");
	return this->str;
}

AbstractVmInstance* ExtArrayObject::toVmInstance(VirtualMachine *vm) {
	SmartContract* contract = vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	AnalyzedType* atype = new AnalyzedType(this->atype); __STP(atype);
	atype->setDim(this->dim);

	if(this->fqn != nullptr){
		UnicodeString* packageName = TypeResolver::getPackageName(this->fqn); __STP(packageName);
		PackageSpace* space = actx->getPackegeSpace(packageName);
		ExceptionThrower<VmClassNotFoundException>::throwExceptionIfCondition(space == nullptr, L"The package was not found.", __FILE__, __LINE__);

		UnicodeString* className = TypeResolver::getClassName(this->fqn); __STP(className);
		AnalyzedClass* aclass = space->getClass(className);
		ExceptionThrower<VmClassNotFoundException>::throwExceptionIfCondition(aclass == nullptr, L"The class was not found.", __FILE__, __LINE__);

		atype->setAnalyzedClass(aclass);
	}

	VmArrayInstance* inst = new(vm) VmArrayInstance(vm, this->length, *atype); __STP(inst);

	int maxLoop = this->array.size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExtObject* exObject = this->array.get(i);

		AbstractVmInstance* vmValue = exObject->toVmInstance(vm);
		IAbstractVmInstanceSubstance* substance = vmValue->getInstance();

		AbstractReference* ref = substance->wrap(inst, vm);
		inst->setReference(vm, i, ref);
	}

	return __STP_MV(inst);
	// FIXME toVmInstance
}

} /* namespace alinous */
