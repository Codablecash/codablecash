/*
 * InterModuleAccessException.cpp
 *
 *  Created on: Dec 6, 2025
 *      Author: iizuka
 */

#include "inter_modular_access/InterModuleAccessException.h"

#include "base/UnicodeString.h"

#include "vm/vm_ctrl/ExecControlManager.h"

#include "vm/VirtualMachine.h"

#include "instance/reserved_classes/ReservedClassRegistory.h"

#include "instance/instance_exception_class/ExceptionInstanceFactory.h"
#include "instance/instance_exception_class/ExceptionClassDeclare.h"
#include "instance/instance_exception_class/VmExceptionInstance.h"

#include "instance/VmClassInstance.h"

#include "engine/sc_analyze/IVmInstanceFactory.h"
#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/AnalyzedClass.h"

#include "lang/sc_declare/ClassExtends.h"


namespace codablecash {

const UnicodeString InterModuleAccessException::NAME{L"InterModuleAccessException"};
const UnicodeString InterModuleAccessException::FULL_QUALIFIED_NAME{L"lang.InterModuleAccessException"};

InterModuleAccessException::InterModuleAccessException() : AbstractExceptionClassDeclare() {
	addDefaultConstructor(&NAME);

	this->name = new UnicodeString(&NAME);

	this->extends = new ClassExtends();
	this->extends->setClassName(&ExceptionClassDeclare::NAME);
}

InterModuleAccessException::~InterModuleAccessException() {

}

AnalyzedClass* InterModuleAccessException::createAnalyzedClass() noexcept {
	InterModuleAccessException* classDec = new InterModuleAccessException();
	AnalyzedClass* aclass = new AnalyzedClass(classDec);

	return aclass;
}

void InterModuleAccessException::throwException(const UnicodeString *msg, VirtualMachine *vm, const CodeElement *element) noexcept {
	ExecControlManager* ctrl = vm->getCtrl();
	IVmInstanceFactory* factory = ExceptionInstanceFactory::getInstance();

	UnicodeString fqn(AbstractExceptionClassDeclare::PACKAGE_NAME);
	fqn.append(L".");
	fqn.append(&NAME);

	AnalyzedClass* aclass = vm->getReservedClassRegistory()->getAnalyzedClass(&fqn);

	VmClassInstance* inst = factory->createInstance(aclass, vm);
	inst->init(vm);


	VmExceptionInstance* exception = dynamic_cast<VmExceptionInstance*>(inst);

	vm->throwException(exception, element);
}

ClassDeclare* InterModuleAccessException::getBaseClass() const noexcept {
	AnalyzedType* atype = this->extends->getAnalyzedType();
	AnalyzedClass* aclass = atype->getAnalyzedClass();

	return aclass->getClassDeclare();
}

IVmInstanceFactory* InterModuleAccessException::getFactory() const noexcept {
	return ExceptionInstanceFactory::getInstance();
}

} /* namespace codablecash */
