/*
 * NullArgument.cpp
 *
 *  Created on: 2019/11/17
 *      Author: iizuka
 */

#include "ext_arguments/NullArgument.h"
#include "ext_arguments/ArgumentType.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "base_io/ByteBuffer.h"

#include "bc_base/BinaryUtils.h"

using namespace codablecash;

namespace alinous {

NullArgument::NullArgument(const NullArgument &inst) : AbstractFunctionExtArguments(ARG_NULL) {
	this->atype = inst.atype != nullptr ? new ArgumentType(*inst.atype) : nullptr;
}

NullArgument::NullArgument() : AbstractFunctionExtArguments(ARG_NULL) {
	this->atype = nullptr;
}

NullArgument::NullArgument(const AnalyzedType* type) : AbstractFunctionExtArguments(ARG_NULL) {
	this->atype = new ArgumentType(type);
}

NullArgument::~NullArgument() {
	delete this->atype;
}

ArgumentType* NullArgument::getType() const noexcept {
	return new ArgumentType(*this->atype);
}

AbstractVmInstance* NullArgument::interpret(VirtualMachine* vm) {
	return nullptr;
}

AbstractFunctionExtArguments* NullArgument::copy() const {
	return new NullArgument(*this);
}

int NullArgument::binarySize() const {
	BinaryUtils::checkNotNull(this->atype);

	int total = sizeof(uint8_t); // argType

	total += this->atype->binarySize();

	return total;
}

void NullArgument::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->atype);

	out->put(this->argType);
	this->atype->toBinary(out);
}

void NullArgument::fromBinary(ByteBuffer *in) {
	this->atype = ArgumentType::createFromBinary(in);
}

} /* namespace alinous */
