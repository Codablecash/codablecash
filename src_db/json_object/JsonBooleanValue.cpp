/*
 * JsonBooleanValue.cpp
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#include "json_object/JsonBooleanValue.h"

#include "base_io/ByteBuffer.h"

#include "ext_arguments/BoolArgument.h"

namespace codablecash {

JsonBooleanValue::JsonBooleanValue(const JsonBooleanValue &inst) {
	this->value = inst.value;
}

JsonBooleanValue::JsonBooleanValue() {
	this->value = false;
}

JsonBooleanValue::~JsonBooleanValue() {

}

AbstractJsonObject* JsonBooleanValue::copy() const noexcept {
	return new JsonBooleanValue(*this);
}

bool JsonBooleanValue::equals(const AbstractJsonObject *other) const noexcept {
	const JsonBooleanValue* v = dynamic_cast<const JsonBooleanValue*>(other);

	return v != nullptr && this->value == v->value;
}

int JsonBooleanValue::binarySize() const {
	int total = sizeof(uint8_t);
	total += sizeof(uint8_t);

	return total;
}

void JsonBooleanValue::toBinary(ByteBuffer *out) const {
	out->put(getType());
	out->put(this->value ? 1 : 0);
}

void JsonBooleanValue::fromBinary(ByteBuffer *in) {
	uint8_t bl = in->get();
	this->value = bl > 0;
}

AbstractFunctionExtArguments* JsonBooleanValue::toFunctionExtArgument() const {
	return new BoolArgument(this->value);
}

} /* namespace codablecash */
