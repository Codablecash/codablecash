/*
 * JsonBooleanValue.h
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#ifndef JSON_OBJECT_JSONBOOLEANVALUE_H_
#define JSON_OBJECT_JSONBOOLEANVALUE_H_

#include "json_object/AbstractJsonValue.h"

namespace codablecash {

class JsonBooleanValue : public AbstractJsonValue {
public:
	JsonBooleanValue(const JsonBooleanValue& inst);
	JsonBooleanValue();
	virtual ~JsonBooleanValue();

	virtual uint8_t getType() const noexcept {
		return JSON_TYPE_BOOLEAN_VALUE;
	}

	bool isValue() const {
		return value;
	}

	void setValue(bool value) {
		this->value = value;
	}

	virtual AbstractJsonObject* copy() const noexcept;
	virtual bool equals(const AbstractJsonObject* other) const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer *out) const;
	virtual void fromBinary(ByteBuffer *in);

	virtual AbstractFunctionExtArguments* toFunctionExtArgument() const;

private:
	bool value;
};

} /* namespace codablecash */

#endif /* JSON_OBJECT_JSONBOOLEANVALUE_H_ */
