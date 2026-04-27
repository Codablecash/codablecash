/*
 * ModularProxyMethodDeclare.cpp
 *
 *  Created on: Dec 4, 2025
 *      Author: iizuka
 */

#include "inter_modular_access/ModularProxyMethodDeclare.h"

#include "lang/sc_declare/MethodDeclare.h"
#include "lang/sc_declare/AccessControlDeclare.h"
#include "lang/sc_declare/ArgumentsListDeclare.h"
#include "lang/sc_declare_types/AbstractType.h"

#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/AnalyzedClass.h"

#include "vm/stack/MethodArgumentSetupper.h"
#include "vm/stack/StackPopper.h"

#include "vm/VirtualMachine.h"

#include "vm/variable_access/FunctionArguments.h"

#include "inter_modular_access/ModuleProxyInstance.h"
#include "inter_modular_access/ModularProxyClassDeclare.h"
#include "inter_modular_access/InterModuleAccessException.h"

#include "smartcontract_instance/InstanceDependencyContext.h"
#include "smartcontract_instance/AbstractExecutableModuleInstance.h"

#include "smartcontract_executor/SmartcontractExecResult.h"

#include "instance/instance_ref/ObjectReference.h"

#include "instance/instance_exception_class/VmExceptionInstance.h"

#include "ext_binary/AbstractExtObject.h"
#include "ext_binary/ExtExceptionObject.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"


namespace codablecash {

ModularProxyMethodDeclare::ModularProxyMethodDeclare() : AbstractReservedMethodDeclare(METHOD_MODULAR_INTERFACE) {

}

ModularProxyMethodDeclare::~ModularProxyMethodDeclare() {

}

void ModularProxyMethodDeclare::init(VirtualMachine *vm) {
}

void ModularProxyMethodDeclare::interpret(FunctionArguments *args, VirtualMachine *vm) {
	MethodArgumentSetupper argSetup(args, vm);

	// stack
	vm->markStackbyMethod(this);
	vm->newStack();
	StackPopper stackPopper(vm);

	doInterpret(args, vm);
}

void ModularProxyMethodDeclare::doInterpret(FunctionArguments *args, VirtualMachine *vm) {
	VmStack* stack = vm->topStack();

	VmClassInstance* _this = args->getThisPtr();
	ModuleProxyInstance* proxyInstance = dynamic_cast<ModuleProxyInstance*>(_this);
	assert(proxyInstance != nullptr);

	AnalyzedClass* aclass = proxyInstance->getAnalyzedClass();
	ClassDeclare* dec = aclass->getClassDeclare();

	ModularProxyClassDeclare* proxyClass = dynamic_cast<ModularProxyClassDeclare*>(dec);
	InstanceDependencyContext* dctx = proxyClass->getInstanceDependencyContext();

	AbstractExecutableModuleInstance* moduleInstance = dctx->getModuleInstance();

	AbstractExtObject* exceptionEx = moduleInstance->invokeMainObjectMethodProxy(vm, this->name, args); __STP(exceptionEx);
	if(exceptionEx != nullptr){
		ExtExceptionObject* exExtObject = dynamic_cast<ExtExceptionObject*>(exceptionEx);

		const UnicodeString* message = exExtObject->getMessage();
		InterModuleAccessException::throwException(message, vm, this);
	}
}

const UnicodeString* ModularProxyMethodDeclare::toString() {
	return this->name;
}

ModularProxyMethodDeclare* ModularProxyMethodDeclare::fromMethodDeclare(MethodDeclare *method) {
	ModularProxyMethodDeclare* dec = new ModularProxyMethodDeclare(); __STP(dec);

	{
		const UnicodeString* name = method->getName();
		dec->setName(new UnicodeString(name));
	}

	{
		AccessControlDeclare* acdec = method->getAccessControlDeclare();
		AccessControlDeclare* copy = dynamic_cast<AccessControlDeclare*>(acdec->binaryCopy());
		dec->setAccessControl(copy);
	}
	{
		AbstractType* type = method->getType();
		AbstractType* copy = dynamic_cast<AbstractType*>(type->binaryCopy());
		dec->setType(copy);
	}
	{
		ArgumentsListDeclare* args = method->getArguments();
		ArgumentsListDeclare* copy = dynamic_cast<ArgumentsListDeclare*>(args->binaryCopy());
		dec->setArguments(copy);
	}

	return __STP_MV(dec);
}

} /* namespace codablecash */
