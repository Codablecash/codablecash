/*
 * AbstractReservedGenericsClassDeclare.cpp
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#include "instance/reserved_generics/AbstractReservedGenericsClassDeclare.h"
#include "instance/reserved_generics/ReservedGeneratedGenericsClassDeclare.h"

#include "instance/reserved_classes/list/ListClassDeclare.h"

#include "filestore_block/exceptions.h"

#include "lang/sc_declare/GenericsParameter.h"
#include "lang/sc_declare/ClassDeclareBlock.h"
#include "lang/sc_declare/ClassExtends.h"
#include "lang/sc_declare/ClassImplements.h"

#include "lang/sc_declare/MethodDeclare.h"
#include "lang/sc_declare/MemberVariableDeclare.h"

#include "base/StackRelease.h"
#include "base/ArrayList.h"
#include "base/UnicodeString.h"

#include "engine/sc_analyze/GeneratedGenericsTypeNaming.h"


namespace alinous {


AbstractReservedGenericsClassDeclare::AbstractReservedGenericsClassDeclare() : GenericsClassDeclare(RESERVED_GENERICS_CLASS_DECLARE) {
	this->methods = new ArrayList<MethodDeclare>();
	this->members = new ArrayList<MemberVariableDeclare>();
}

AbstractReservedGenericsClassDeclare::~AbstractReservedGenericsClassDeclare() {
	this->methods->deleteElements();
	delete this->methods;

	this->members->deleteElements();
	delete this->members;
}

AbstractReservedGenericsClassDeclare* AbstractReservedGenericsClassDeclare::createFromBinary(ByteBuffer *in) {
	AbstractReservedGenericsClassDeclare* ret = nullptr;
	uint16_t classType = in->getShort();

	switch(classType){
	case TYPE_GENERICS_LIST_CLASS:
		ret = new ListClassDeclare();
		break;
	default:
		throw new BinaryFormatException(__FILE__, __LINE__);
	}

	return ret;
}

int AbstractReservedGenericsClassDeclare::binarySize() const {
	int total = sizeof(uint16_t);
	total += sizeof(uint16_t); // generics type

	return total;
}

void AbstractReservedGenericsClassDeclare::toBinary(ByteBuffer *out) const {
	toBinaryHead(out);
	out->putShort(getGenericsClassType()); // generics type
}

void AbstractReservedGenericsClassDeclare::fromBinary(ByteBuffer *in) {

}

void AbstractReservedGenericsClassDeclare::preAnalyze(AnalyzeContext *actx) {
	// GenericsParams
	int maxLoop = this->genericsParams->size();
	for(int i = 0; i != maxLoop; ++i){
		GenericsParameter* param = this->genericsParams->get(i);

		param->setParent(this);
		param->preAnalyze(actx);
	}
}

void AbstractReservedGenericsClassDeclare::analyzeTypeRef(AnalyzeContext *actx) {
	int maxLoop = this->genericsParams->size();
	for(int i = 0; i != maxLoop; ++i){
		GenericsParameter* param = this->genericsParams->get(i);

		param->analyzeTypeRef(actx);
	}
}

void AbstractReservedGenericsClassDeclare::analyze(AnalyzeContext *actx) {
}

void AbstractReservedGenericsClassDeclare::addMethod(MethodDeclare *method) noexcept {
	this->methods->addElement(method);
}

void AbstractReservedGenericsClassDeclare::doGenerateGenericsImplement(
		ReservedGeneratedGenericsClassDeclare *inst, HashMap<UnicodeString, AbstractType> *input) const {
	inst->copyCodePositions(this);
	inst->setFactory(getFactory());

	// interface
	inst->setInterface(this->interface);

	// name
	UnicodeString* name = GeneratedGenericsTypeNaming::fromGenericsClassDeclare(this, input);
	inst->setName(name);

	// extends
	if(this->extends != nullptr){
		ClassExtends* ex = this->extends->generateGenericsImplement(input);
		inst->setExtends(ex);
	}

	// implements
	if(this->implements != nullptr){
		ClassImplements* implements = this->implements->generateGenericsImplement(input);
		inst->setImplements(implements);
	}
}

} /* namespace alinous */
