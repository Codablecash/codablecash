/*
 * ListClassDeclare.cpp
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#include "instance/reserved_classes/list/ListClassDeclare.h"
#include "instance/reserved_classes/list/ListClassInstanceFactory.h"
#include "instance/reserved_classes/list/ListListMethod.h"

#include "instance/reserved_generics/ReservedGeneratedGenericsClassDeclare.h"

#include "base/UnicodeString.h"
#include "base/ArrayList.h"
#include "base/StackRelease.h"

#include "engine/sc_analyze/AnalyzedClass.h"

#include "lang/sc_declare/GenericsParameter.h"
#include "lang/sc_declare/ClassName.h"
#include "lang/sc_declare/MemberVariableDeclare.h"


namespace alinous {

const UnicodeString ListClassDeclare::NAME(L"List");
const UnicodeString ListClassDeclare::PACKAGE(L"lang");

ListClassDeclare::ListClassDeclare() {
	this->fqn = new UnicodeString(&PACKAGE);
	this->fqn->append(L".").append(&NAME);

	this->name = new UnicodeString(&NAME);

	// generic parameter
	{
		GenericsParameter *p = new GenericsParameter();
		p->setGenericsName(new UnicodeString(L"T"));
		p->setGenericsExtendsName(new ClassName(L"Object"));
		this->genericsParams->addElement(p);
	}

	addMethod(new ListListMethod());
}

ListClassDeclare::~ListClassDeclare() {

}

const UnicodeString* ListClassDeclare::getPackageName() const noexcept {
	return &PACKAGE;
}

IVmInstanceFactory* ListClassDeclare::getFactory() const noexcept {
	return ListClassInstanceFactory::getInstance();
}

const UnicodeString* ListClassDeclare::getFullQualifiedName() noexcept {
	return this->fqn;
}

AnalyzedClass* ListClassDeclare::createAnalyzedClass() noexcept {
	ListClassDeclare* classDec = new ListClassDeclare();
	AnalyzedClass* aclass = new AnalyzedClass(classDec);

	return aclass;
}

ClassDeclare* ListClassDeclare::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) {
	ReservedGeneratedGenericsClassDeclare* inst = new ReservedGeneratedGenericsClassDeclare(); __STP(inst);
	inst->setGenericsClassDeclare(this);

	AbstractReservedGenericsClassDeclare::doGenerateGenericsImplement(inst, input);

	int maxLoop = this->methods->size();
	for(int i = 0; i != maxLoop; ++i){
		MethodDeclare* m = this->methods->get(i);

		MethodDeclare* copied = m->generateGenericsImplement(input);
		inst->addMethod(copied);
	}

	maxLoop = this->members->size();
	for(int i = 0; i != maxLoop; ++i){
		MemberVariableDeclare* m = this->members->get(i);

		MemberVariableDeclare* copied = m->generateGenericsImplement(input);
		inst->addMemberVariable(copied);
	}

	return __STP_MV(inst);
}

} /* namespace alinous */
