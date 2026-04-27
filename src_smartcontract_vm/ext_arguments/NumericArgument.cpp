/*
 * NumericArgument.cpp
 *
 *  Created on: 2019/11/17
 *      Author: iizuka
 */

#include "ext_arguments/NumericArgument.h"
#include "ext_arguments/ArgumentType.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "vm/VirtualMachine.h"

#include "instance/instance_ref/PrimitiveReference.h"

#include "base_io/ByteBuffer.h"

#include "bc_base/BinaryUtils.h"

using namespace codablecash;

namespace alinous {

NumericArgument::NumericArgument() : AbstractFunctionExtArguments(ARG_NUMERIC) {
	this->data = 0;
	this->atype = nullptr;
}

NumericArgument::NumericArgument(int64_t value, uint8_t atype) : AbstractFunctionExtArguments(ARG_NUMERIC) {
	this->data = value;
	this->atype = new ArgumentType();
	this->atype->setType(atype);
}

NumericArgument::~NumericArgument() {
	delete this->atype;
}

ArgumentType* NumericArgument::getType() const noexcept {
	return new ArgumentType(*this->atype);
}

AbstractVmInstance* NumericArgument::interpret(VirtualMachine* vm) {
	uint8_t type = this->atype->getType();

	PrimitiveReference* ref = nullptr;

	switch(type){
	case AnalyzedType::TYPE_BYTE:
		ref = PrimitiveReference::createByteReference(vm, this->data);
		break;
	case AnalyzedType::TYPE_CHAR:
		ref = PrimitiveReference::createCharReference(vm, this->data);
		break;
	case AnalyzedType::TYPE_SHORT:
		ref = PrimitiveReference::createShortReference(vm, this->data);
		break;
	case AnalyzedType::TYPE_LONG:
		ref = PrimitiveReference::createLongReference(vm, this->data);
		break;
	case AnalyzedType::TYPE_INT:
	default:
		ref = PrimitiveReference::createIntReference(vm, this->data);
		break;
	}

	return ref;
}

AbstractFunctionExtArguments* NumericArgument::copy() const {
	return new NumericArgument(this->data, this->atype->getType());
}

int NumericArgument::binarySize() const {
	BinaryUtils::checkNotNull(this->atype);

	int total = sizeof(uint8_t); // argType

	total += sizeof(int64_t);
	total += this->atype->binarySize();

	return total;
}

void NumericArgument::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->atype);

	out->put(this->argType);

	out->putLong(this->data);
	this->atype->toBinary(out);
}

void NumericArgument::fromBinary(ByteBuffer *in) {
	this->data = in->getLong();
	this->atype = ArgumentType::createFromBinary(in);
}

} /* namespace alinous */
