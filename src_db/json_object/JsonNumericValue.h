/*
 * JsonNumericValue.h
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#ifndef JSON_OBJECT_JSONNUMERICVALUE_H_
#define JSON_OBJECT_JSONNUMERICVALUE_H_

#include "json_object/AbstractJsonValue.h"

namespace codablecash {

class JsonNumericValue : public AbstractJsonValue {
public:
	JsonNumericValue(const JsonNumericValue& inst);
	explicit JsonNumericValue(int v);
	JsonNumericValue();
	virtual ~JsonNumericValue();

	virtual uint8_t getType() const noexcept {
		return JSON_TYPE_NUMERIC_VALUE;
	}

	void setValue(int v) {
		this->value = v;
	}
	int getValue() const noexcept {
		return this->value;
	}

	virtual AbstractJsonObject* copy() const noexcept;
	virtual bool equals(const AbstractJsonObject* other) const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer *out) const;
	virtual void fromBinary(ByteBuffer *in);

	virtual AbstractFunctionExtArguments* toFunctionExtArgument() const;

private:
	int value;
};

} /* namespace codablecash */

#endif /* JSON_OBJECT_JSONNUMERICVALUE_H_ */
