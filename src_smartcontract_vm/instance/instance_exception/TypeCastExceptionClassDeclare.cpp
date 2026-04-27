/*
 * TypeCastException.cpp
 *
 *  Created on: 2020/04/23
 *      Author: iizuka
 */

#include "instance/instance_exception/TypeCastExceptionClassDeclare.h"

#include "base/UnicodeString.h"

#include "engine/sc_analyze/AnalyzedClass.h"
#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/IVmInstanceFactory.h"

#include "instance/instance_exception_class/ExceptionInstanceFactory.h"
#include "instance/instance_exception_class/ExceptionClassDeclare.h"
#include "instance/instance_exception_class/VmExceptionInstance.h"

#include "lang/sc_declare/ClassExtends.h"

#include "vm/vm_ctrl/ExecControlManager.h"

#include "vm/VirtualMachine.h"

#include "instance/reserved_classes/ReservedClassRegistory.h"

namespace alinous {

const UnicodeString TypeCastExceptionClassDeclare::NAME{L"TypeCastException"};
const UnicodeString TypeCastExceptionClassDeclare::FULL_QUALIFIED_NAME{L"lang.TypeCastException"};

TypeCastExceptionClassDeclare::TypeCastExceptionClassDeclare() {
	addDefaultConstructor(&NAME);

	this->name = new UnicodeString(&NAME);

	this->extends = new ClassExtends();
	this->extends->setClassName(&ExceptionClassDeclare::NAME);
}

AnalyzedClass* TypeCastExceptionClassDeclare::createAnalyzedClass() noexcept {
	TypeCastExceptionClassDeclare* classDec = new TypeCastExceptionClassDeclare();
	AnalyzedClass* aclass = new AnalyzedClass(classDec);

	return aclass;
}

void TypeCastExceptionClassDeclare::throwException(VirtualMachine* vm, const CodeElement* element) noexcept {
	throwException(true, vm, element);
}

void TypeCastExceptionClassDeclare::throwException(bool cond,VirtualMachine *vm, const CodeElement *element) noexcept {
	if(cond){
		ExecControlManager* ctrl = vm->getCtrl();
		IVmInstanceFactory* factory = ExceptionInstanceFactory::getInstance();

		UnicodeString fqn(&AbstractExceptionClassDeclare::PACKAGE_NAME);
		fqn.append(L".");
		fqn.append(&NAME);
		AnalyzedClass* aclass = vm->getReservedClassRegistory()->getAnalyzedClass(&fqn);

		VmClassInstance* inst = factory->createInstance(aclass, vm);
		inst->init(vm);

		VmExceptionInstance* exception = dynamic_cast<VmExceptionInstance*>(inst);

		vm->throwException(exception, element);
	}
}

TypeCastExceptionClassDeclare::~TypeCastExceptionClassDeclare() {
}

ClassDeclare* TypeCastExceptionClassDeclare::getBaseClass() const noexcept {
	AnalyzedType* atype = this->extends->getAnalyzedType();
	AnalyzedClass* aclass = atype->getAnalyzedClass();

	return aclass->getClassDeclare();
}

IVmInstanceFactory* TypeCastExceptionClassDeclare::getFactory() const noexcept {
	return ExceptionInstanceFactory::getInstance();
}

} /* namespace alinous */
