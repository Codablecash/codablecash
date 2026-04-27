/*
 * NullLiteral.cpp
 *
 *  Created on: 2019/02/05
 *      Author: iizuka
 */

#include "lang/sc_expression_literal/NullLiteral.h"
#include "engine/sc_analyze/AnalyzedType.h"

namespace alinous {

NullLiteral::NullLiteral() : AbstractExpression(CodeElement::EXP_NULL_LITERAL) {
}

NullLiteral::~NullLiteral() {
}

void NullLiteral::preAnalyze(AnalyzeContext* actx) {

}

void NullLiteral::analyzeTypeRef(AnalyzeContext* actx) {
}


void NullLiteral::analyze(AnalyzeContext* actx) {

}

int NullLiteral::binarySize() const {
	int total = sizeof(uint16_t);

	total += positionBinarySize();

	return total;
}

void NullLiteral::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::EXP_NULL_LITERAL);

	positionToBinary(out);
}

void NullLiteral::fromBinary(ByteBuffer* in) {
	positionFromBinary(in);
}

AnalyzedType NullLiteral::getType(AnalyzeContext* actx) {
	return AnalyzedType(AnalyzedType::TYPE_NULL);
}

void NullLiteral::init(VirtualMachine* vm) {

}

AbstractVmInstance* NullLiteral::interpret(VirtualMachine* vm) {
	return nullptr;
}

AbstractExpression* NullLiteral::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	NullLiteral* inst = new NullLiteral();
	inst->copyCodePositions(this);

	return inst;
}

} /* namespace alinous */
