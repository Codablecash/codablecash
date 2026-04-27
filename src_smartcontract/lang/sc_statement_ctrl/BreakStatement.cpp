/*
 * BreakStatement.cpp
 *
 *  Created on: 2019/02/05
 *      Author: iizuka
 */

#include "lang/sc_statement_ctrl/BreakStatement.h"

#include "vm/vm_ctrl/ExecControlManager.h"
#include "vm/vm_ctrl/BreakControl.h"

#include "vm/VirtualMachine.h"


namespace alinous {

BreakStatement::BreakStatement() : AbstractStatement(CodeElement::STMT_BREAK) {

}

BreakStatement::~BreakStatement() {
}

void BreakStatement::preAnalyze(AnalyzeContext* actx) {
}

void BreakStatement::analyzeTypeRef(AnalyzeContext* actx) {
}

void BreakStatement::analyze(AnalyzeContext* actx) {
}

void BreakStatement::init(VirtualMachine* vm) {
}

int BreakStatement::binarySize() const {
	int total = sizeof(uint16_t);

	total += positionBinarySize();

	return total;
}

void BreakStatement::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::STMT_BREAK);

	positionToBinary(out);
}

void BreakStatement::fromBinary(ByteBuffer* in) {
	positionFromBinary(in);
}

void BreakStatement::interpret(VirtualMachine* vm) {
	ExecControlManager* ctrl = vm->getCtrl();
	BreakControl* retCtrl = new BreakControl();

	ctrl->setInstruction(retCtrl);
}

bool BreakStatement::hasCtrlStatement() const noexcept {
	return true;
}

AbstractStatement* BreakStatement::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	BreakStatement* inst = new BreakStatement();
	inst->copyCodePositions(this);

	return inst;
}

} /* namespace alinous */
