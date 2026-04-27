/*
 * ArgumentDeclare.cpp
 *
 *  Created on: 2019/01/16
 *      Author: iizuka
 */

#include "lang/sc_declare/ArgumentDeclare.h"
#include "lang/sc_declare_types/AbstractType.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/ValidationError.h"
#include "engine/sc_analyze/AnalyzedType.h"

#include "base/UnicodeString.h"


namespace alinous {

ArgumentDeclare::ArgumentDeclare() : CodeElement(CodeElement::ARGUMENT_DECLARE) {
	this->type = nullptr;
	this->name = nullptr;
	this->atype = nullptr;
}

ArgumentDeclare::~ArgumentDeclare() {
	delete this->type;
	delete this->name;
	delete this->atype;
}

void ArgumentDeclare::setType(AbstractType* type) noexcept {
	this->type = type;
}

void ArgumentDeclare::setName(UnicodeString* name) noexcept {
	this->name = name;
}

const UnicodeString* ArgumentDeclare::getName() const noexcept {
	return this->name;
}

void ArgumentDeclare::preAnalyze(AnalyzeContext *actx) {
	this->type->setParent(this);
	this->type->preAnalyze(actx);

	actx->detectGenericsType(this->type);
}

void ArgumentDeclare::analyzeTypeRef(AnalyzeContext* actx) {
	this->type->analyzeTypeRef(actx);

	TypeResolver* typeResolver = actx->getTypeResolver();

	this->atype = typeResolver->resolveType(this, this->type);
	if(this->atype == nullptr){
		actx->addValidationError(ValidationError::CODE_WRONG_TYPE_NAME, this, L"The type '{0}' does not exists.", {this->type->toString()});
	}
	else if(this->atype->getType() == AnalyzedType::TYPE_VOID){
		actx->addValidationError(ValidationError::CODE_WRONG_TYPE_NAME, this, L"Cannot use void for type declare.", {});
	}
}

void ArgumentDeclare::analyze(AnalyzeContext *actx) {
	this->type->analyze(actx);
}

int alinous::ArgumentDeclare::binarySize() const {
	checkNotNull(this->type);
	checkNotNull(this->name);

	int total = sizeof(uint16_t);
	total += this->type->binarySize();
	total += stringSize(this->name);

	total += positionBinarySize();

	return total;
}

void alinous::ArgumentDeclare::toBinary(ByteBuffer* out) const {
	checkNotNull(this->type);
	checkNotNull(this->name);

	out->putShort(CodeElement::ARGUMENT_DECLARE);
	this->type->toBinary(out);
	putString(out, this->name);

	positionToBinary(out);
}

AnalyzedType* ArgumentDeclare::getAnalyzedType() const noexcept {
	return this->atype;
}

void alinous::ArgumentDeclare::fromBinary(ByteBuffer* in) {
	CodeElement* element = createFromBinary(in);
	checkIsType(element);
	checkNotNull(element);
	this->type = dynamic_cast<AbstractType*>(element);

	this->name = getString(in);

	positionFromBinary(in);
}

ArgumentDeclare* ArgumentDeclare::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	ArgumentDeclare* inst = new ArgumentDeclare();
	inst->copyCodePositions(this);

	AbstractType* copiedType = this->type->generateGenericsImplement(input);
	inst->setType(copiedType);

	inst->setName(new UnicodeString(this->name));

	return inst;
}

} /* namespace alinous */
