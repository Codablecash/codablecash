/*
 * ObjectClassDeclare.cpp
 *
 *  Created on: Nov 2, 2025
 *      Author: iizuka
 */

#include "instance/reserved_classes/object/ObjectClassDeclare.h"
#include "instance/reserved_classes/object/ObjectClassInstanceFactory.h"
#include "instance/reserved_classes/object/ObjectObjectMethod.h"

#include "base/UnicodeString.h"

#include "base_io/ByteBuffer.h"

#include "engine/sc_analyze/AnalyzedClass.h"

namespace alinous {

const UnicodeString ObjectClassDeclare::NAME(L"Object");
const UnicodeString ObjectClassDeclare::PACKAGE(L"lang");

ObjectClassDeclare::ObjectClassDeclare() {
	this->fqn = new UnicodeString(&PACKAGE);
	this->fqn->append(L".").append(&NAME);

	this->name = new UnicodeString(&NAME);

	addMethod(new ObjectObjectMethod());
}

ObjectClassDeclare::~ObjectClassDeclare() {
	delete this->fqn;
}

AnalyzedClass* ObjectClassDeclare::createAnalyzedClass() noexcept {
	ObjectClassDeclare* classDec = new ObjectClassDeclare();
	AnalyzedClass* aclass = new AnalyzedClass(classDec);

	return aclass;
}


const UnicodeString* ObjectClassDeclare::getName() const noexcept {
	return &NAME;
}

const UnicodeString* ObjectClassDeclare::getPackageName() const noexcept {
	return &PACKAGE;
}

const UnicodeString* ObjectClassDeclare::getFullQualifiedName() noexcept {
	return this->fqn;
}

void ObjectClassDeclare::init(VirtualMachine *vm) {

}

ClassDeclare* ObjectClassDeclare::getBaseClass() const noexcept {
	return nullptr;
}

IVmInstanceFactory* ObjectClassDeclare::getFactory() const noexcept {
	return ObjectClassInstanceFactory::getInstance();
}

} /* namespace alinous */
