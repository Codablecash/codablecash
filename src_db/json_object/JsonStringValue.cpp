/*
 * JsonStringValue.cpp
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#include "json_object/JsonStringValue.h"

#include "base/UnicodeString.h"

#include "bc_base/BinaryUtils.h"

#include "base_io/ByteBuffer.h"

#include "ext_arguments/StringArgument.h"

namespace codablecash {

JsonStringValue::JsonStringValue(const JsonStringValue &inst) {
	this->value = new UnicodeString(inst.value);
}

JsonStringValue::JsonStringValue() {
	this->value = nullptr;
}

JsonStringValue::JsonStringValue(const wchar_t *str) {
	this->value = new UnicodeString(str);
}

JsonStringValue::~JsonStringValue() {
	delete this->value;
}

void JsonStringValue::setValue(const UnicodeString *value) noexcept {
	delete this->value;
	this->value = new UnicodeString(value);
}

AbstractJsonObject* JsonStringValue::copy() const noexcept {
	return new JsonStringValue(*this);
}

bool JsonStringValue::equals(const AbstractJsonObject *other) const noexcept {
	const JsonStringValue* otherString = dynamic_cast<const JsonStringValue*>(other);

	const UnicodeString* str = nullptr;
	if(otherString != nullptr) {
		str = otherString->getValue();
	}

	return otherString != nullptr && this->value->equals(str);
}

int JsonStringValue::binarySize() const {
	BinaryUtils::checkNotNull(this->value);

	int total = sizeof(uint8_t);
	total += BinaryUtils::stringSize(this->value);

	return total;
}

void JsonStringValue::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->value);

	out->put(getType());
	BinaryUtils::putString(out, this->value);
}

void JsonStringValue::fromBinary(ByteBuffer *in) {
	this->value = BinaryUtils::getString(in);
	BinaryUtils::checkNotNull(this->value);
}

AbstractFunctionExtArguments* JsonStringValue::toFunctionExtArgument() const {
	return new StringArgument(this->value);
}

} /* namespace codablecash */
