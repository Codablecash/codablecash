/*
 * BlankStatement.cpp
 *
 *  Created on: 2019/02/15
 *      Author: iizuka
 */

#include "lang/sc_statement/BlankStatement.h"

namespace alinous {

BlankStatement::BlankStatement() : AbstractStatement(CodeElement::STMT_BLANK) {
}

BlankStatement::~BlankStatement() {
}

void BlankStatement::preAnalyze(AnalyzeContext* actx) {
}
void BlankStatement::analyzeTypeRef(AnalyzeContext* actx) {
}
void BlankStatement::analyze(AnalyzeContext* actx) {
}

void alinous::BlankStatement::init(VirtualMachine* vm) {
}

int BlankStatement::binarySize() const {
	int total = sizeof(uint16_t);

	total += positionBinarySize();

	return total;
}

void BlankStatement::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::STMT_BLANK);

	positionToBinary(out);
}

void BlankStatement::fromBinary(ByteBuffer* in) {
	positionFromBinary(in);
}

void BlankStatement::interpret(VirtualMachine* vm) {
}

bool BlankStatement::hasCtrlStatement() const noexcept {
	return false;
}

AbstractStatement* BlankStatement::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	BlankStatement* inst = new BlankStatement();
	inst->copyCodePositions(this);

	return inst;
}

} /* namespace alinous */
