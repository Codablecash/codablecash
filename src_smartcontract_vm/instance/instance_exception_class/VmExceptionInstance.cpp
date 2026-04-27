/*
 * VmExceptionInstance.cpp
 *
 *  Created on: 2020/04/14
 *      Author: iizuka
 */

#include "instance/instance_exception_class/VmExceptionInstance.h"
#include "instance/instance_exception_class/StackTraceElement.h"

#include "instance/VmInstanceTypesConst.h"

#include "ext_binary/ExtExceptionObject.h"

#include "engine/sc_analyze/AnalyzedClass.h"

#include "base/UnicodeString.h"

#include "instance/instance_string/VmString.h"

namespace alinous {

VmExceptionInstance::VmExceptionInstance(AnalyzedClass* clazz, VirtualMachine* vm)
				: VmClassInstance(VmInstanceTypesConst::INST_EXCEPTION, clazz, vm) {
	this->element = nullptr;
	this->message = nullptr;
	this->stacktrace = new(vm) VMemList<StackTraceElement>(vm);
}

VmExceptionInstance::~VmExceptionInstance() {
	this->element = nullptr;
	delete this->message;

	delete this->stacktrace;
}

void VmExceptionInstance::setCodeElement(const CodeElement* element) noexcept {
	this->element = element;
}

const CodeElement* VmExceptionInstance::getElement() const noexcept {
	return this->element;
}

AbstractExtObject* VmExceptionInstance::toClassExtObject(const UnicodeString* name, VTableRegistory* reg) {
	ExtExceptionObject* extObj = new ExtExceptionObject(name);
	extObj->setCodeElement(getElement());
	extObj->setClassName(this->clazz->getFullQualifiedName());

	if(this->message != nullptr){
		UnicodeString message(this->message->towString());
		extObj->setMessage(&message);
	}

	return extObj;
}

void VmExceptionInstance::addStacktrace(StackTraceElement *stack) {
	this->stacktrace->addElement(stack);
}

void VmExceptionInstance::setMessage(const UnicodeString *message, VirtualMachine *vm) {
	delete this->message;
	this->message = new(vm) VmString(vm, message);
}

} /* namespace alinous */
