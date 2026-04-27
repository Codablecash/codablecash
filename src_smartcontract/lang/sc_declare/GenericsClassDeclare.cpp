/*
 * GenericsClassDeclare.cpp
 *
 *  Created on: Oct 18, 2025
 *      Author: iizuka
 */

#include "lang/sc_declare/GenericsClassDeclare.h"
#include "lang/sc_declare/GenericsParameter.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/PackageSpace.h"
#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/ValidationError.h"
#include "engine/sc_analyze/GeneratedGenericsTypeNaming.h"

#include "engine/sc/CompilationUnit.h"

#include "lang/sc_declare/GenericsGeneratedClassDeclare.h"
#include "lang/sc_declare/ClassExtends.h"
#include "lang/sc_declare/ClassImplements.h"
#include "lang/sc_declare/ClassDeclareBlock.h"


namespace alinous {

GenericsClassDeclare::GenericsClassDeclare(short kind) : ClassDeclare(kind) {
	this->genericsParams = new ArrayList<GenericsParameter>();
}

// GENERICS_CLASS_DECLARE
GenericsClassDeclare::GenericsClassDeclare() : ClassDeclare(GENERICS_CLASS_DECLARE) {
	this->genericsParams = new ArrayList<GenericsParameter>();
}

GenericsClassDeclare::~GenericsClassDeclare() {
	this->genericsParams->deleteElements();
	delete this->genericsParams;
}

void GenericsClassDeclare::addGenericsParameter(GenericsParameter *param) noexcept {
	this->genericsParams->addElement(param);
}

int GenericsClassDeclare::binarySize() const {
	checkNotNull(this->genericsParams);

	int total = ClassDeclare::binarySize();

	int maxLoop = this->genericsParams->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		GenericsParameter* param = this->genericsParams->get(i);
		total += param->binarySize();
	}

	return total;
}

void GenericsClassDeclare::toBinary(ByteBuffer *out) const {
	checkNotNull(this->genericsParams);

	ClassDeclare::toBinary(out);

	int maxLoop = this->genericsParams->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		GenericsParameter* param = this->genericsParams->get(i);
		param->toBinary(out);
	}
}

void GenericsClassDeclare::fromBinary(ByteBuffer *in) {
	ClassDeclare::fromBinary(in);

	int maxLoop = in->getShort();

	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = CodeElement::createFromBinary(in); __STP(element);
		GenericsParameter* param = dynamic_cast<GenericsParameter*>(element);
		checkNotNull(param);

		this->genericsParams->addElement(param);

		__STP_MV(element);
	}
}

void GenericsClassDeclare::preAnalyze(AnalyzeContext *actx) {
	CompilationUnit* unit = getCompilationUnit();
	PackageSpace* space = actx->getPackegeSpace(unit->getPackageName());

	AnalyzedClass* dec = space->getClass(this->name);
	if(dec != nullptr){
		actx->addValidationError(ValidationError::CODE_CLASS_ALREADY_EXISTS, this, L"Class {0} is already registered", {this->name});

		return;
	}

	space->addClassDeclare(this);

	// GenericsParams
	int maxLoop = this->genericsParams->size();
	for(int i = 0; i != maxLoop; ++i){
		GenericsParameter* param = this->genericsParams->get(i);

		param->setParent(this);
		param->preAnalyze(actx);
	}
}

void GenericsClassDeclare::analyzeTypeRef(AnalyzeContext *actx) {
//	ClassDeclare::analyzeTypeRef(actx);

	int maxLoop = this->genericsParams->size();
	for(int i = 0; i != maxLoop; ++i){
		GenericsParameter* param = this->genericsParams->get(i);

		param->analyzeTypeRef(actx);
	}
}

void GenericsClassDeclare::analyze(AnalyzeContext *actx) {
	//ClassDeclare::analyze(actx);
}

const GenericsParameter* GenericsClassDeclare::findGenericsType(const UnicodeString* name) const noexcept {
	GenericsParameter* retParam = nullptr;

	int maxLoop = this->genericsParams->size();
	for(int i = 0; i != maxLoop; ++i){
		GenericsParameter* param = this->genericsParams->get(i);
		const UnicodeString* gname = param->getGenericsName();

		if(name->equals(gname)){
			retParam = param;
			break;
		}
	}

	return retParam;
}

ClassDeclare* GenericsClassDeclare::generateGenericsImplement(HashMap<UnicodeString, AbstractType>* input) {
	GenericsGeneratedClassDeclare* clazz = new GenericsGeneratedClassDeclare();
	clazz->copyCodePositions(this);
	clazz->setGenericsClassDeclare(this);

	// interface
	clazz->setInterface(this->interface);

	// block
	if(this->block != nullptr){
		ClassDeclareBlock* copiedblock = this->block->generateGenericsImplement(input);
		clazz->setBlock(copiedblock);
	}

	// name
	UnicodeString* name = GeneratedGenericsTypeNaming::fromGenericsClassDeclare(this, input);
	clazz->setName(name);

	// extends
	if(this->extends != nullptr){
		ClassExtends* ex = this->extends->generateGenericsImplement(input);
		clazz->setExtends(ex);
	}

	// implements
	if(this->implements != nullptr){
		ClassImplements* implements = this->implements->generateGenericsImplement(input);
		clazz->setImplements(implements);
	}

	return clazz;
}

void GenericsClassDeclare::init(VirtualMachine *vm) {
}

} /* namespace alinous */
