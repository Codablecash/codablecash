/*
 * LongType.cpp
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#include "lang/sc_declare_types/LongType.h"
#include "lang/sc_declare_types/ITypeVisitor.h"

#include "base/UnicodeString.h"


namespace alinous {

const UnicodeString LongType::TYPE_NAME(L"long");

LongType::LongType() : AbstractPrimitiveType(CodeElement::TYPE_LONG) {
}

LongType::~LongType() {
}

int LongType::binarySize() const {
	int total = sizeof(uint16_t);
	total += AbstractType::binarySize();

	total += positionBinarySize();

	return total;
}

void LongType::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::TYPE_LONG);
	AbstractType::toBinary(out);

	positionToBinary(out);
}

void LongType::fromBinary(ByteBuffer* in) {
	AbstractType::fromBinary(in);

	positionFromBinary(in);
}

const UnicodeString* LongType::toString() noexcept {
	return &TYPE_NAME;
}

AbstractType* LongType::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	LongType* inst = new LongType();
	inst->copyCodePositions(this);
	return inst;
}

void LongType::visit(ITypeVisitor *visitor) {
	visitor->visit(this);
	visitor->exit(this);
}

} /* namespace alinous */
