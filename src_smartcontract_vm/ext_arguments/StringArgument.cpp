/*
 * StringArgument.cpp
 *
 *  Created on: 2019/11/17
 *      Author: iizuka
 */

#include "ext_arguments/StringArgument.h"
#include "ext_arguments/ArgumentType.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "vm/VirtualMachine.h"

#include "base/UnicodeString.h"

#include "instance/instance_string/VmStringInstance.h"

#include "bc_base/BinaryUtils.h"

#include "base_io/ByteBuffer.h"

namespace alinous {

StringArgument::StringArgument() : AbstractFunctionExtArguments(ARG_STRING) {
	this->value = nullptr;
}

StringArgument::StringArgument(const UnicodeString* str) : AbstractFunctionExtArguments(ARG_STRING) {
	this->value = new UnicodeString(str);
}

StringArgument::~StringArgument() {
	delete this->value;
}

ArgumentType* StringArgument::getType() const noexcept {
	AnalyzedType strtype(AnalyzedType::TYPE_STRING);

	return new ArgumentType(&strtype);
}

AbstractVmInstance* StringArgument::interpret(VirtualMachine* vm) {
	VmStringInstance* inst = new(vm) VmStringInstance(vm, this->value);

	return inst;
}

AbstractFunctionExtArguments* StringArgument::copy() const {
	return new StringArgument(this->value);
}

int StringArgument::binarySize() const {
	BinaryUtils::checkNotNull(this->value);

	int total = sizeof(uint8_t); // argType
	total += BinaryUtils::stringSize(this->value);

	return total;
}

void StringArgument::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->value);

	out->put(this->argType);
	BinaryUtils::putString(out, this->value);
}

void StringArgument::fromBinary(ByteBuffer *in) {
	this->value = BinaryUtils::getString(in);
}

} /* namespace alinous */
