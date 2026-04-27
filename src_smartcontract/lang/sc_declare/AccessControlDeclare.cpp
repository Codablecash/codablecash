/*
 * AccessControlDeclare.cpp
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#include "lang/sc_declare/AccessControlDeclare.h"

namespace alinous {

AccessControlDeclare::AccessControlDeclare(char accessCtrl) : CodeElement(CodeElement::ACCESS_CONTROL_DECLARE) {
	this->accessCtrl = accessCtrl;
}

AccessControlDeclare::AccessControlDeclare() : CodeElement(CodeElement::ACCESS_CONTROL_DECLARE) {
	this->accessCtrl = 0;
}

AccessControlDeclare::~AccessControlDeclare() {
}

void AccessControlDeclare::setCtrl(char ctrl) noexcept {
	this->accessCtrl = ctrl;
}

int AccessControlDeclare::binarySize() const {
	int total = sizeof(uint16_t);
	total += sizeof(char);

	total += positionBinarySize();

	return total;
}

void AccessControlDeclare::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::ACCESS_CONTROL_DECLARE);
	out->put(this->accessCtrl);

	positionToBinary(out);
}

void AccessControlDeclare::fromBinary(ByteBuffer* in) {
	this->accessCtrl = in->get();

	positionFromBinary(in);
}

AccessControlDeclare* AccessControlDeclare::generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const {
	AccessControlDeclare* inst = new AccessControlDeclare();
	inst->copyCodePositions(this);

	inst->setCtrl(this->accessCtrl);

	return inst;
}

} /* namespace alinous */
