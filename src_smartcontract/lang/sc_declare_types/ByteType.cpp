/*
 * ByteType.cpp
 *
 *  Created on: 2019/05/24
 *      Author: iizuka
 */

#include "lang/sc_declare_types/ByteType.h"
#include "lang/sc_declare_types/ITypeVisitor.h"

#include "base/UnicodeString.h"


namespace alinous {

const UnicodeString ByteType::TYPE_NAME(L"byte");

ByteType::ByteType() : AbstractPrimitiveType(CodeElement::TYPE_BYTE) {
}

ByteType::~ByteType() {
}

int ByteType::binarySize() const {
	int total = sizeof(uint16_t);
	total += AbstractType::binarySize();

	total += positionBinarySize();

	return total;
}

void ByteType::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::TYPE_BYTE);
	AbstractType::toBinary(out);

	positionToBinary(out);
}

void ByteType::fromBinary(ByteBuffer* in) {
	AbstractType::fromBinary(in);

	positionFromBinary(in);
}

const UnicodeString* ByteType::toString() noexcept {
	return &TYPE_NAME;
}

AbstractType* ByteType::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	ByteType* inst = new ByteType();
	inst->copyCodePositions(this);
	return inst;
}

void ByteType::visit(ITypeVisitor *visitor) {
	visitor->visit(this);
	visitor->exit(this);
}

} /* namespace alinous */
