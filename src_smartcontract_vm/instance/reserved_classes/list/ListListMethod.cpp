/*
 * ListListMethod.cpp
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#include "instance/reserved_classes/list/ListListMethod.h"

#include "base/UnicodeString.h"

#include "lang/sc_declare/AccessControlDeclare.h"
#include "lang/sc_declare/ArgumentsListDeclare.h"

#include "vm/VirtualMachine.h"

namespace alinous {

const UnicodeString ListListMethod::METHOD_NAME(L"List");

ListListMethod::ListListMethod() : AbstractGenericReservedMethodDeclare(METHOD_LIST_LIST) {
	this->_static = false;
	this->name = new UnicodeString(METHOD_NAME);
	this->ctrl = new AccessControlDeclare(AccessControlDeclare::PUBLIC);
	this->type = nullptr;
	this->args = new ArgumentsListDeclare();
}

ListListMethod::~ListListMethod() {

}

void ListListMethod::init(VirtualMachine *vm) {
	AbstractReservedMethodDeclare::init(vm);
}

void ListListMethod::interpret(FunctionArguments *args, VirtualMachine *vm) {
	AbstractReservedMethodDeclare::interpret(args, vm);

}

const UnicodeString* ListListMethod::toString() {
	return &METHOD_NAME;
}

MethodDeclare* ListListMethod::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	ListListMethod* inst = new ListListMethod();
	inst->copyCodePositions(this);

	AbstractGenericReservedMethodDeclare::doGenerateGenericsImplement(inst, input);

	return inst;
}

} /* namespace alinous */
