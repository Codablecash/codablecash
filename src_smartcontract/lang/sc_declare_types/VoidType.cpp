/*
 * VoidType.cpp
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#include "lang/sc_declare_types/VoidType.h"
#include "lang/sc_declare_types/ITypeVisitor.h"

#include "base/UnicodeString.h"


namespace alinous {

const UnicodeString VoidType::TYPE_NAME(L"void");

VoidType::VoidType() : AbstractType(CodeElement::TYPE_VOID) {

}

VoidType::~VoidType() {
}

int VoidType::binarySize() const {
	int total = sizeof(uint16_t);

	total += positionBinarySize();

	return total;
}

void VoidType::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::TYPE_VOID);

	positionToBinary(out);
}

void VoidType::fromBinary(ByteBuffer* in) {
	positionFromBinary(in);
}

const UnicodeString* VoidType::toString() noexcept {
	return &TYPE_NAME;
}

AbstractType* VoidType::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	VoidType* inst = new VoidType();
	inst->copyCodePositions(this);
	return inst;
}

void VoidType::visit(ITypeVisitor *visitor) {
	visitor->visit(this);
	visitor->exit(this);
}

} /* namespace alinous */
