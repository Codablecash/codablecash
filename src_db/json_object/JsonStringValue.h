/*
 * JsonStringValue.h
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#ifndef JSON_OBJECT_JSONSTRINGVALUE_H_
#define JSON_OBJECT_JSONSTRINGVALUE_H_

#include "json_object/AbstractJsonValue.h"

namespace alinous {
class UnicodeString;
}
using namespace alinous;


namespace codablecash {

class JsonStringValue : public AbstractJsonValue {
public:
	JsonStringValue(const JsonStringValue& inst);
	JsonStringValue();
	explicit JsonStringValue(const wchar_t* str);
	virtual ~JsonStringValue();

	virtual uint8_t getType() const noexcept {
		return JSON_TYPE_STRING_VALUE;
	}

	void setValue(const UnicodeString* value) noexcept;
	const UnicodeString* getValue() const noexcept {
		return this->value;
	}

	virtual AbstractJsonObject* copy() const noexcept;
	virtual bool equals(const AbstractJsonObject* other) const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer *out) const;
	virtual void fromBinary(ByteBuffer *in);

	virtual AbstractFunctionExtArguments* toFunctionExtArgument() const;

private:
	UnicodeString* value;
};

} /* namespace codablecash */

#endif /* JSON_OBJECT_JSONSTRINGVALUE_H_ */
