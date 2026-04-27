/*
 * JsonNumericValue.cpp
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#include "json_object/JsonNumericValue.h"

#include "base_io/ByteBuffer.h"

#include "ext_arguments/NumericArgument.h"

#include "engine/sc_analyze/AnalyzedType.h"


namespace codablecash {

JsonNumericValue::JsonNumericValue(const JsonNumericValue &inst) {
	this->value = inst.value;
}

JsonNumericValue::JsonNumericValue() {
	this->value = 0;
}

JsonNumericValue::JsonNumericValue(int v) {
	this->value = v;
}

JsonNumericValue::~JsonNumericValue() {

}

AbstractJsonObject* JsonNumericValue::copy() const noexcept {
	return new JsonNumericValue(*this);
}

bool JsonNumericValue::equals(const AbstractJsonObject *other) const noexcept {
	const JsonNumericValue* v = dynamic_cast<const JsonNumericValue*>(other);

	return v != nullptr && this->value == v->value;
}

int JsonNumericValue::binarySize() const {
	int total = sizeof(uint8_t);
	total += sizeof(uint32_t);

	return total;
}

void JsonNumericValue::toBinary(ByteBuffer *out) const {
	out->put(getType());
	out->putInt(this->value);
}

void JsonNumericValue::fromBinary(ByteBuffer *in) {
	this->value = in->getInt();
}

AbstractFunctionExtArguments* JsonNumericValue::toFunctionExtArgument() const {
	return new NumericArgument(this->value, AnalyzedType::TYPE_INT);
}

} /* namespace codablecash */
