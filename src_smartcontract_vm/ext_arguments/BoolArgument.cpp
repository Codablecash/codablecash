/*
 * BoolArgument.cpp
 *
 *  Created on: 2020/03/19
 *      Author: iizuka
 */

#include "ext_arguments/BoolArgument.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "vm/VirtualMachine.h"

#include "instance/instance_ref/PrimitiveReference.h"

#include "base_io/ByteBuffer.h"

#include "ext_arguments/ArgumentType.h"

namespace alinous {

BoolArgument::BoolArgument() : AbstractFunctionExtArguments(ARG_BOOL) {
	this->value = false;
}

BoolArgument::BoolArgument(bool value) : AbstractFunctionExtArguments(ARG_BOOL) {
	this->value = value;
}

BoolArgument::~BoolArgument() {

}

ArgumentType* BoolArgument::getType() const noexcept {
	AnalyzedType atype(AnalyzedType::TYPE_BOOL);
	return new ArgumentType(&atype);
}

AbstractVmInstance* BoolArgument::interpret(VirtualMachine* vm) {
	PrimitiveReference* inst = PrimitiveReference::createBoolReference(vm, this->value);

	return inst;
}

AbstractFunctionExtArguments* BoolArgument::copy() const {
	return new BoolArgument(this->value);
}

int BoolArgument::binarySize() const {
	int total = sizeof(uint8_t); // argType
	total += sizeof(uint8_t);

	return total;

}

void BoolArgument::toBinary(ByteBuffer *out) const {
	out->put(this->argType);

	out->put(this->value ? 1 : 0);
}

void BoolArgument::fromBinary(ByteBuffer *in) {
	uint8_t bl = in->get();
	this->value = bl > 0;
}

} /* namespace alinous */
