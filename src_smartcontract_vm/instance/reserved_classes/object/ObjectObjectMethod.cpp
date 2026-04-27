/*
 * ObjectObject.cpp
 *
 *  Created on: Nov 3, 2025
 *      Author: iizuka
 */

#include "instance/reserved_classes/object/ObjectObjectMethod.h"

#include "base/UnicodeString.h"

#include "vm/stack/MethodArgumentSetupper.h"
#include "vm/stack/StackPopper.h"

#include "vm/VirtualMachine.h"

#include "lang/sc_declare/AccessControlDeclare.h"
#include "lang/sc_declare/ArgumentsListDeclare.h"


namespace alinous {

const UnicodeString ObjectObjectMethod::METHOD_NAME(L"Object");

ObjectObjectMethod::ObjectObjectMethod() : AbstractReservedMethodDeclare(METHOD_OBJECT_OBJECT) {
	this->_static = false;
	this->name = new UnicodeString(METHOD_NAME);
	this->ctrl = new AccessControlDeclare(AccessControlDeclare::PUBLIC);
	this->type = nullptr;
	this->args = new ArgumentsListDeclare();
}

ObjectObjectMethod::~ObjectObjectMethod() {

}

void ObjectObjectMethod::init(VirtualMachine *vm) {
	AbstractReservedMethodDeclare::init(vm);
}

void ObjectObjectMethod::interpret(FunctionArguments *args, VirtualMachine *vm) {
	AbstractReservedMethodDeclare::interpret(args, vm);

}

const UnicodeString* ObjectObjectMethod::toString() {
	return &METHOD_NAME;
}

} /* namespace alinous */
