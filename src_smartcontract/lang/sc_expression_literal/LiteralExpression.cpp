/*
 * LiteralExpression.cpp
 *
 *  Created on: 2019/02/05
 *      Author: iizuka
 */

#include "lang/sc_expression_literal/LiteralExpression.h"
#include "base/UnicodeString.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "instance/instance_ref/VmRootReference.h"
#include "instance/instance_string/VmStringInstance.h"

#include "vm/VirtualMachine.h"

#include "base/StackRelease.h"

namespace alinous {

LiteralExpression::LiteralExpression() : AbstractExpression(CodeElement::EXP_LITERAL){
	this->str = nullptr;
	this->dquote = true;
	this->reference = nullptr;
}

LiteralExpression::~LiteralExpression() {
	delete this->str;
	this->reference = nullptr;
}

void LiteralExpression::preAnalyze(AnalyzeContext* actx) {

}

void LiteralExpression::analyzeTypeRef(AnalyzeContext* actx) {

}

void LiteralExpression::analyze(AnalyzeContext* actx) {

}

void LiteralExpression::setString(UnicodeString* str, bool dquote) noexcept {
	this->str = str;
	this->dquote = dquote;
}

UnicodeString* LiteralExpression::getStringBody() const noexcept {
	UnicodeString* invalue = this->str->substring(1, this->str->length() - 1);
	return invalue;
}


int LiteralExpression::binarySize() const {
	checkNotNull(this->str);

	int total = sizeof(uint16_t);
	total += sizeof(uint8_t);
	total += stringSize(this->str);

	total += positionBinarySize();

	return total;
}

void LiteralExpression::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::EXP_LITERAL);

	out->put(this->dquote ? 1 : 0);
	putString(out, this->str);

	positionToBinary(out);
}

void LiteralExpression::fromBinary(ByteBuffer* in) {
	char bl = in->get();
	this->dquote = (bl == 1);
	this->str = getString(in);

	positionFromBinary(in);
}

AnalyzedType LiteralExpression::getType(AnalyzeContext* actx) {
	return AnalyzedType(AnalyzedType::TYPE_STRING);
}

void LiteralExpression::init(VirtualMachine* vm) {
	VmRootReference* rootRef = vm->getVmRootReference();


	UnicodeString* invalue = this->str->substring(1, this->str->length() - 1); __STP(invalue);

	this->reference = rootRef->newStringConstReferenece(rootRef, invalue, vm);
}

AbstractVmInstance* LiteralExpression::interpret(VirtualMachine* vm) {
	return this->reference;
}

AbstractExpression* LiteralExpression::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	LiteralExpression* inst = new LiteralExpression();
	inst->copyCodePositions(this);

	inst->setString(new UnicodeString(this->str), this->dquote);

	return inst;
}

} /* namespace alinous */
