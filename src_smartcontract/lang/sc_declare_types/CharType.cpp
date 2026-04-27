/*
 * CharType.cpp
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#include "lang/sc_declare_types/CharType.h"
#include "lang/sc_declare_types/ITypeVisitor.h"

#include "base/UnicodeString.h"


namespace alinous {

const UnicodeString CharType::TYPE_NAME(L"char");

CharType::CharType() : AbstractPrimitiveType(CodeElement::TYPE_CHAR) {
}

CharType::~CharType() {
}

int CharType::binarySize() const {
	int total = sizeof(uint16_t);
	total += AbstractType::binarySize();

	total += positionBinarySize();

	return total;
}

void CharType::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::TYPE_CHAR);
	AbstractType::toBinary(out);

	positionToBinary(out);
}

void CharType::fromBinary(ByteBuffer* in) {
	AbstractType::fromBinary(in);

	positionFromBinary(in);
}

const UnicodeString* CharType::toString() noexcept {
	return &TYPE_NAME;
}

AbstractType* CharType::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	CharType* inst = new CharType();
	inst->copyCodePositions(this);
	return inst;
}

void CharType::visit(ITypeVisitor *visitor) {
	visitor->visit(this);
	visitor->exit(this);
}

} /* namespace alinous */
