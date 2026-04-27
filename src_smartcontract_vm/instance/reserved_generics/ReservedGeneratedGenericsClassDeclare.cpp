/*
 * ReservedGeneratedGenericsClassDeclare.cpp
 *
 *  Created on: Nov 7, 2025
 *      Author: iizuka
 */

#include "instance/reserved_generics/ReservedGeneratedGenericsClassDeclare.h"

#include "lang/sc_declare/MethodDeclare.h"
#include "lang/sc_declare/MemberVariableDeclare.h"
#include "lang/sc_declare/ClassExtends.h"
#include "lang/sc_declare/ClassImplements.h"

#include "base/StackRelease.h"

namespace alinous {


ReservedGeneratedGenericsClassDeclare::ReservedGeneratedGenericsClassDeclare()
		: GenericsGeneratedClassDeclare(RESERVED_GENERATED_GENERICS_CLASS_DECLARE) {
	this->methods = new ArrayList<MethodDeclare>();
	this->members = new ArrayList<MemberVariableDeclare>();
}

ReservedGeneratedGenericsClassDeclare::~ReservedGeneratedGenericsClassDeclare() {
	this->methods->deleteElements();
	this->members->deleteElements();

	delete this->methods;
	delete this->members;
}

int ReservedGeneratedGenericsClassDeclare::binarySize() const {
	checkNotNull(this->name);

	int total = sizeof(uint16_t); // this->kind

	total += sizeof(uint8_t); // interface

	total += stringSize(this->name);

	total += sizeof(uint8_t);
	if(this->extends != nullptr){
		total += this->extends->binarySize();
	}

	total += sizeof(uint8_t);
	if(this->implements != nullptr){
		total += this->implements->binarySize();
	}

	// members
	int maxLoop = this->methods->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		MethodDeclare* m = this->methods->get(i);
		total += m->binarySize();
	}

	maxLoop = this->members->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		MemberVariableDeclare* m = this->members->get(i);
		total += m->binarySize();
	}

	return total;

}

void ReservedGeneratedGenericsClassDeclare::toBinary(ByteBuffer *out) {
	out->put(this->interface ? 1 : 0);

	checkNotNull(this->name);

	out->putShort(this->kind);

	putString(out, this->name);

	uint8_t bl = this->extends != nullptr ? 1 : 0;
	out->put(bl);
	if(this->extends != nullptr){
		this->extends->toBinary(out);
	}

	bl = this->implements != nullptr ? 1 : 0;
	out->put(bl);
	if(this->implements != nullptr){
		this->implements->toBinary(out);
	}

	// members
	int maxLoop = this->methods->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		MethodDeclare* m = this->methods->get(i);
		m->toBinary(out);
	}

	maxLoop = this->members->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		MemberVariableDeclare* m = this->members->get(i);
		m->toBinary(out);
	}
}

void ReservedGeneratedGenericsClassDeclare::fromBinary(ByteBuffer *in) {
	this->interface = (in->get() > 0 ? 1 : 0);
	this->name = getString(in);

	bool bl = in->get();
	if(bl == 1){
		CodeElement* element = CodeElement::createFromBinary(in);
		checkKind(element, CodeElement::CLASS_EXTENDS);
		this->extends = dynamic_cast<ClassExtends*>(element);
	}

	bl = in->get();
	if(bl == 1){
		CodeElement* element = CodeElement::createFromBinary(in);
		checkKind(element, CodeElement::CLASS_IMPLEMENTS);
		this->implements = dynamic_cast<ClassImplements*>(element);
	}

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = CodeElement::createFromBinary(in); __STP(element);
		MethodDeclare* m = dynamic_cast<MethodDeclare*>(element);
		checkNotNull(m);

		__STP_MV(element);
		this->methods->addElement(m);
	}

	maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = CodeElement::createFromBinary(in); __STP(element);
		MemberVariableDeclare* m = dynamic_cast<MemberVariableDeclare*>(element);
		checkNotNull(m);

		__STP_MV(element);
		this->members->addElement(m);
	}
}

void ReservedGeneratedGenericsClassDeclare::preAnalyze(AnalyzeContext *actx) {
	GenericsGeneratedClassDeclare::preAnalyze(actx);

}

void ReservedGeneratedGenericsClassDeclare::analyzeTypeRef(AnalyzeContext *actx) {
	GenericsGeneratedClassDeclare::analyzeTypeRef(actx);
}

void ReservedGeneratedGenericsClassDeclare::analyze(AnalyzeContext *actx) {
	GenericsGeneratedClassDeclare::analyze(actx);
}

void ReservedGeneratedGenericsClassDeclare::addMethod(MethodDeclare *method) noexcept {
	this->methods->addElement(method);
}

void ReservedGeneratedGenericsClassDeclare::addMemberVariable(MemberVariableDeclare *variable) noexcept {
	this->members->addElement(variable);
}

} /* namespace alinous */
