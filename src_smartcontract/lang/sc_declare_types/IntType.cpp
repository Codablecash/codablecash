/*
 * IntType.cpp
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#include "lang/sc_declare_types/IntType.h"
#include "lang/sc_declare_types/ITypeVisitor.h"

#include "base/UnicodeString.h"


namespace alinous {

const UnicodeString IntType::TYPE_NAME(L"int");

IntType::IntType()  : AbstractPrimitiveType(CodeElement::TYPE_INT) {

}

IntType::~IntType() {
}


int IntType::binarySize() const {
	int total = sizeof(uint16_t);
	total += AbstractType::binarySize();

	total += positionBinarySize();

	return total;
}

void IntType::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::TYPE_INT);
	AbstractType::toBinary(out);

	positionToBinary(out);
}

void IntType::fromBinary(ByteBuffer* in) {
	AbstractType::fromBinary(in);

	positionFromBinary(in);
}

const UnicodeString* IntType::toString() noexcept {
	return &TYPE_NAME;
}

AbstractType* IntType::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	IntType* inst = new IntType();
	inst->copyCodePositions(this);
	return inst;
}

void IntType::visit(ITypeVisitor *visitor) {
	visitor->visit(this);
	visitor->exit(this);
}

} /* namespace alinous */
