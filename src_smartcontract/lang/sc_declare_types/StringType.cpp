/*
 * StringType.cpp
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#include "lang/sc_declare_types/StringType.h"
#include "lang/sc_declare_types/ITypeVisitor.h"

#include "base/UnicodeString.h"


namespace alinous {

const UnicodeString StringType::TYPE_NAME(L"String");

StringType::StringType() : AbstractType(CodeElement::TYPE_STRING) {
}

StringType::~StringType() {
}

int StringType::binarySize() const {
	int total = sizeof(uint16_t);
	total += AbstractType::binarySize();

	total += positionBinarySize();

	return total;
}

void StringType::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::TYPE_STRING);
	AbstractType::toBinary(out);

	positionToBinary(out);
}

void StringType::fromBinary(ByteBuffer* in) {
	AbstractType::fromBinary(in);

	positionFromBinary(in);
}

const UnicodeString* StringType::toString() noexcept {
	return &TYPE_NAME;
}

AbstractType* StringType::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	StringType* inst = new StringType();
	inst->copyCodePositions(this);
	return inst;
}

void StringType::visit(ITypeVisitor *visitor) {
	visitor->visit(this);
	visitor->exit(this);
}

} /* namespace alinous */
