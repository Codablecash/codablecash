/*
 * ExtExceptionObject.cpp
 *
 *  Created on: 2020/04/16
 *      Author: iizuka
 */

#include "ext_binary/ExtExceptionObject.h"

#include "instance/VmInstanceTypesConst.h"

#include "instance/instance_exception_class/VmExceptionInstance.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "engine/sc/SmartContract.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/PackageSpace.h"

#include "vm/exceptions.h"

#include "bc/ExceptionThrower.h"


using namespace codablecash;

namespace alinous {

ExtExceptionObject::ExtExceptionObject(const UnicodeString* name) : AbstractExtObject(name, VmInstanceTypesConst::INST_EXCEPTION) {
	this->element = nullptr;
	this->className = nullptr;
	this->message = nullptr;
}

ExtExceptionObject::~ExtExceptionObject() {
	this->element = nullptr;
	delete this->className;
	delete this->message;
}

AbstractExtObject* ExtExceptionObject::copy() const noexcept {
	ExtExceptionObject* newObj = new ExtExceptionObject(this->name);
	newObj->element = this->element;
	newObj->className = this->className == nullptr ? nullptr : new UnicodeString(this->className);
	newObj->message = this->message == nullptr ? nullptr : new UnicodeString(this->message);

	return newObj;
}

void ExtExceptionObject::setCodeElement(const CodeElement* element) noexcept {
	this->element = element;
}

const CodeElement* ExtExceptionObject::getCodeElement() const noexcept {
	return this->element;
}

void ExtExceptionObject::setClassName(const UnicodeString* className) noexcept {
	this->className = new UnicodeString(className);
}

const UnicodeString* ExtExceptionObject::getClassName() const noexcept {
	return this->className;
}

void ExtExceptionObject::setMessage(const UnicodeString *message) noexcept {
	this->message = new UnicodeString(message);
}

const UnicodeString* ExtExceptionObject::toString() const noexcept {
	delete this->str;
	this->str = new UnicodeString(L"@");

	this->str->append(this->className);

	return this->str;
}

const UnicodeString* ExtExceptionObject::getMessage() const noexcept {
	return this->message;
}

AbstractVmInstance* ExtExceptionObject::toVmInstance(VirtualMachine *vm) {
	SmartContract* contract = vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();

	UnicodeString* packageName = TypeResolver::getPackageName(this->className); __STP(packageName);
	PackageSpace* space = actx->getPackegeSpace(packageName);
	ExceptionThrower<VmClassNotFoundException>::throwExceptionIfCondition(space == nullptr, L"The package was not found.", __FILE__, __LINE__);

	UnicodeString* className = TypeResolver::getClassName(this->className); __STP(className);
	AnalyzedClass* aclass = space->getClass(className);
	ExceptionThrower<VmClassNotFoundException>::throwExceptionIfCondition(aclass == nullptr, L"The class was not found.", __FILE__, __LINE__);

	VmExceptionInstance* vmInstance = new(vm) VmExceptionInstance(aclass, vm); __STP(vmInstance);
	if(this->message != nullptr){
		vmInstance->setMessage(this->message, vm);
	}

	return __STP_MV(vmInstance);
	// FIXME toVmInstance
}

} /* namespace alinous */
