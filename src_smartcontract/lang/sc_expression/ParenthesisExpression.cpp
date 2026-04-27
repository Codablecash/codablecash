/*
 * ParenthesisExpression.cpp
 *
 *  Created on: 2019/02/05
 *      Author: iizuka
 */

#include "lang/sc_expression/ParenthesisExpression.h"
#include "engine/sc_analyze/AnalyzedType.h"

namespace alinous {

ParenthesisExpression::ParenthesisExpression() : AbstractExpression(CodeElement::EXP_PARENTHESIS) {
	this->exp = nullptr;
}

ParenthesisExpression::~ParenthesisExpression() {
	delete this->exp;
}

void ParenthesisExpression::preAnalyze(AnalyzeContext* actx) {
	this->exp->setParent(this);
	this->exp->preAnalyze(actx);
}

void ParenthesisExpression::analyzeTypeRef(AnalyzeContext* actx) {
	this->exp->analyzeTypeRef(actx);
}

void ParenthesisExpression::analyze(AnalyzeContext* actx) {
	this->exp->analyze(actx);
}

void ParenthesisExpression::setExp(AbstractExpression* exp) noexcept {
	this->exp = exp;
}

int ParenthesisExpression::binarySize() const {
	checkNotNull(this->exp);

	int total = sizeof(uint16_t);
	total += this->exp->binarySize();

	total += positionBinarySize();

	return total;
}

void ParenthesisExpression::toBinary(ByteBuffer* out) const {
	checkNotNull(this->exp);

	out->putShort(CodeElement::EXP_PARENTHESIS);
	this->exp->toBinary(out);

	positionToBinary(out);
}

void ParenthesisExpression::fromBinary(ByteBuffer* in) {
	CodeElement* element = createFromBinary(in);
	checkIsExp(element);
	this->exp = dynamic_cast<AbstractExpression*>(element);

	positionFromBinary(in);
}

AnalyzedType ParenthesisExpression::getType(AnalyzeContext* actx) {
	return this->exp->getType(actx);
}

void ParenthesisExpression::init(VirtualMachine* vm) {
	this->exp->init(vm);
}

AbstractVmInstance* ParenthesisExpression::interpret(VirtualMachine* vm) {
	return this->exp->interpret(vm);
}

AbstractExpression* ParenthesisExpression::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	ParenthesisExpression* inst = new ParenthesisExpression();
	inst->copyCodePositions(this);

	AbstractExpression* copied = this->exp->generateGenericsImplement(input);
	inst->setExp(copied);

	return inst;
}

} /* namespace alinous */
