/*
 * BoolType.cpp
 *
 *  Created on: 2019/06/11
 *      Author: iizuka
 */

#include "lang/sc_declare_types/BoolType.h"
#include "lang/sc_declare_types/ITypeVisitor.h"

#include "base/UnicodeString.h"


namespace alinous {

const UnicodeString BoolType::TYPE_NAME(L"boolean");

BoolType::BoolType() : AbstractPrimitiveType(CodeElement::TYPE_BOOL) {

}

BoolType::~BoolType() {

}

int BoolType::binarySize() const {
	int total = sizeof(uint16_t);
	total += AbstractType::binarySize();

	total += positionBinarySize();

	return total;
}

void BoolType::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::TYPE_BOOL);
	AbstractType::toBinary(out);

	positionToBinary(out);
}

void BoolType::fromBinary(ByteBuffer* in) {
	AbstractType::fromBinary(in);

	positionFromBinary(in);
}

const UnicodeString* BoolType::toString() noexcept {
	return &TYPE_NAME;
}

void BoolType::setType(short kind) noexcept {
	this->kind = kind;
}

AbstractType* BoolType::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	BoolType* inst = new BoolType();
	inst->copyCodePositions(this);
	return inst;
}

void BoolType::visit(ITypeVisitor *visitor) {
	visitor->visit(this);
	visitor->exit(this);
}

} /* namespace alinous */
