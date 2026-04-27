/*
 * JsonValuePair.cpp
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#include "json_object/JsonValuePair.h"
#include "json_object/AbstractJsonValue.h"
#include "json_object/AbstractJsonObject.h"

#include "bc_base/BinaryUtils.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"
namespace codablecash {

JsonValuePair::JsonValuePair(const JsonValuePair& inst) {
	this->key = inst.key != nullptr ? dynamic_cast<AbstractJsonValue*>(inst.key->copy()) : nullptr;
	this->value = inst.value != nullptr ? dynamic_cast<AbstractJsonObject*>(inst.value->copy()) : nullptr;
}

JsonValuePair::JsonValuePair() {
	this->key = nullptr;
	this->value = nullptr;
}

JsonValuePair::~JsonValuePair() {
	delete this->key;
	delete this->value;
}

void JsonValuePair::setKey(AbstractJsonValue *key) noexcept {
	delete this->key;
	this->key = key;
}

void JsonValuePair::setValue(AbstractJsonObject *value) noexcept {
	delete this->value;
	this->value = value;
}

AbstractJsonObject* JsonValuePair::copy() const noexcept {
	return new JsonValuePair(*this);
}

bool JsonValuePair::equals(const AbstractJsonObject *other) const noexcept {
	bool bl = false;
	const JsonValuePair* v = dynamic_cast<const JsonValuePair*>(other);

	if(v != nullptr && this->key->equals(v->key) && this->value->equals(v->value)){
		bl = true;
	}

	return bl;
}

int JsonValuePair::binarySize() const {
	BinaryUtils::checkNotNull(this->key);
	BinaryUtils::checkNotNull(this->value);

	int total = sizeof(uint8_t);

	total += this->key->binarySize();
	total += this->value->binarySize();

	return total;
}

void JsonValuePair::toBinary(ByteBuffer *out) const {
	out->put(getType());

	this->key->toBinary(out);
	this->value->toBinary(out);
}

void JsonValuePair::fromBinary(ByteBuffer *in) {
	{
		AbstractJsonObject* object = createFromBinary(in); __STP(object);
		this->key = dynamic_cast<AbstractJsonValue*>(object);
		BinaryUtils::checkNotNull(this->key);
		__STP_MV(object);
	}

	this->value = createFromBinary(in);
	BinaryUtils::checkNotNull(this->value);
}

} /* namespace codablecash */
