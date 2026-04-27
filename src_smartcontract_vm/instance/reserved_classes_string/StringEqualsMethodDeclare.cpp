/*
 * StringEqualsMethodDeclare.cpp
 *
 *  Created on: Dec 23, 2025
 *      Author: iizuka
 */

#include "instance/reserved_classes_string/StringEqualsMethodDeclare.h"

#include "lang/sc_declare/AccessControlDeclare.h"
#include "lang/sc_declare/ArgumentDeclare.h"
#include "lang/sc_declare/ArgumentsListDeclare.h"

#include "lang/sc_declare_types/StringType.h"
#include "lang/sc_declare_types/BoolType.h"

#include "base/UnicodeString.h"

#include "vm/VirtualMachine.h"

#include "vm/stack/MethodArgumentSetupper.h"
#include "vm/stack/StackPopper.h"

#include "vm/variable_access/FunctionArguments.h"

#include "instance/instance_string/VmStringInstance.h"

#include "instance/instance_exception/NullPointerExceptionClassDeclare.h"
#include "instance/instance_exception/TypeCastExceptionClassDeclare.h"

#include "instance/instance_ref/PrimitiveReference.h"

namespace alinous {

const UnicodeString StringEqualsMethodDeclare::METHOD_NAME(L"equals");

StringEqualsMethodDeclare::StringEqualsMethodDeclare() : AbstractReservedMethodDeclare(METHOD_STRING_EQUALS) {
	AccessControlDeclare* access = new AccessControlDeclare();
	access->setCtrl(AccessControlDeclare::PUBLIC);
	setAccessControl(access);

	setType(new BoolType());
	setName(new UnicodeString(&METHOD_NAME));

	ArgumentsListDeclare* args = new ArgumentsListDeclare();
	setArguments(args);

	{
		ArgumentDeclare* other = new ArgumentDeclare();

		StringType* type = new StringType();
		other->setType(type);
		other->setName(new UnicodeString(L"other"));

		args->addArgument(other);
	}
}

StringEqualsMethodDeclare::~StringEqualsMethodDeclare() {

}

void StringEqualsMethodDeclare::interpret(FunctionArguments *args, VirtualMachine *vm) {
	MethodArgumentSetupper argSetup(args, vm);
	{
		// stack
		vm->markStackbyMethod(this);
		vm->newStack();
		StackPopper stackPopper(vm);

		interpretFunctionArguments(vm);
	}

	const ArrayList<IAbstractVmInstanceSubstance>* list = args->getArguments();

	VmClassInstance* _this = args->getThisPtr();
	IAbstractVmInstanceSubstance* _other = list->get(0);
	NullPointerExceptionClassDeclare::throwException(_other == nullptr, vm, this);

	VmStringInstance* _thisstr = dynamic_cast<VmStringInstance*>(_this);
	VmStringInstance* other = dynamic_cast<VmStringInstance*>(_other);

	TypeCastExceptionClassDeclare::throwException(other == nullptr, vm, this);

	VmString* left = _thisstr->getValue();
	VmString* right = other->getValue();

	int cmp = left->compareTo(right);

	PrimitiveReference* boolRef = PrimitiveReference::createBoolReference(vm, cmp == 0 ? 1 : 0);
	args->setReturnedValue(boolRef);
}

} /* namespace alinous */
