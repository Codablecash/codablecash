/*
 * StackTraceElement.cpp
 *
 *  Created on: Dec 6, 2025
 *      Author: iizuka
 */

#include "instance/instance_exception_class/StackTraceElement.h"

#include "instance/VmInstanceTypesConst.h"


namespace alinous {

StackTraceElement::StackTraceElement(uint64_t serial) : AbstractVmInstance(VmInstanceTypesConst::INST_EXCEPTION_STACK_TRACE, serial) {
	this->currentMethod = nullptr;
	this->entryPoint = nullptr;
}

StackTraceElement::~StackTraceElement() {
	this->currentMethod = nullptr;
	this->entryPoint = nullptr;
}

void StackTraceElement::setCurrentMethod(MethodDeclare *currentMethod) {
	this->currentMethod = currentMethod;
}

void StackTraceElement::setEntryPoint(AbstractExpression *entryPoint) {
	this->entryPoint = entryPoint;
}

IAbstractVmInstanceSubstance* StackTraceElement::getInstance() noexcept {
	return nullptr;
}

int StackTraceElement::valueCompare(const IAbstractVmInstanceSubstance *right) const noexcept {
	return 0;
}

const UnicodeString* StackTraceElement::toString() const noexcept {
	return nullptr;
}

} /* namespace alinous */
