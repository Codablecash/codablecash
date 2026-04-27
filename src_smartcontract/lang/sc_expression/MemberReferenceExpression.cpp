/*
 * MemberReferenceExpression.cpp
 *
 *  Created on: 2019/02/05
 *      Author: iizuka
 */

#include "lang/sc_expression/MemberReferenceExpression.h"

#include "lang/sc_expression/AbstractExpression.h"
#include "lang/sc_expression/VariableIdentifier.h"

#include "engine/sc_analyze/AnalyzedType.h"
#include "engine/sc_analyze/NameSegments.h"
#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/AnalyzeContext.h"

#include "vm/variable_access/VariableInstractionHolder.h"

#include "base/UnicodeString.h"


namespace alinous {

MemberReferenceExpression::MemberReferenceExpression() : AbstractBinaryExpression(CodeElement::EXP_MEMBER_REF) {
}

MemberReferenceExpression::~MemberReferenceExpression() {
}

void MemberReferenceExpression::preAnalyze(AnalyzeContext* actx) {
	AbstractBinaryExpression::preAnalyze(actx);
}

void MemberReferenceExpression::analyzeTypeRef(AnalyzeContext* actx) {
	AbstractBinaryExpression::analyzeTypeRef(actx);
}

void MemberReferenceExpression::analyze(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	VariableInstractionHolder* holder = getVariableInstractionHolder();

	int begin = packageLookAhead(actx);

	bool ex = false;
	for(int i = begin; i != maxLoop; ++i){
		AbstractExpression* exp = this->list.get(i);
		holder->addExpression(exp, actx);

		ex = (ex || exp->throwsException());
	}

	setThrowsException(ex);

	holder->analyze(actx, this);
}

int MemberReferenceExpression::packageLookAhead(AnalyzeContext* actx) {
	TypeResolver* resolver = actx->getTypeResolver();
	VariableInstractionHolder* holder = getVariableInstractionHolder();

	int lookahead = 0;

	NameSegments segments;

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExpression* exp = this->list.get(i);

		if(exp->getKind() == CodeElement::EXP_VARIABLE_ID){
			VariableIdentifier* valId = dynamic_cast<VariableIdentifier*>(exp);
			const UnicodeString* seg = valId->getName();

			segments.addSegment(seg);
		}
		else{
			break;
		}
	}

	// longest match
	while(segments.length() > 1){
		const UnicodeString* fqn = segments.toString();
		AnalyzedType* at = resolver->findClassType(this, fqn);

		if(at != nullptr){
			holder->addFirstClassIdentifier(at);
			delete at;

			lookahead = segments.length();
			break;
		}

		segments.removeTop();
	}

	return lookahead;
}

int MemberReferenceExpression::binarySize() const {
	int total = sizeof(uint16_t);
	total += AbstractBinaryExpression::binarySize();

	total += positionBinarySize();

	return total;
}

void MemberReferenceExpression::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::EXP_MEMBER_REF);
	AbstractBinaryExpression::toBinary(out);

	positionToBinary(out);
}

void MemberReferenceExpression::fromBinary(ByteBuffer* in) {
	AbstractBinaryExpression::fromBinary(in);

	positionFromBinary(in);
}

AnalyzedType MemberReferenceExpression::getType(AnalyzeContext* actx) {
	VariableInstractionHolder* holder = getVariableInstractionHolder();
	AnalyzedType* atype = holder->getAnalyzedType();

	// analyze member ref type
	return *atype;
}

AbstractVmInstance* MemberReferenceExpression::interpret(VirtualMachine* vm) {
	VariableInstractionHolder* holder = getVariableInstractionHolder();

	return holder->interpret(vm);
}

AbstractExpression* MemberReferenceExpression::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	MemberReferenceExpression* inst = new MemberReferenceExpression();
	inst->copyCodePositions(this);
	inst->copyExpressionList(this, input);

	return inst;
}

} /* namespace alinous */
