/*
 * AbstractJsonObject.h
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#ifndef JSON_OBJECT_ABSTRACTJSONOBJECT_H_
#define JSON_OBJECT_ABSTRACTJSONOBJECT_H_
#include <cstdint>

namespace alinous {
class ByteBuffer;
}
using namespace alinous;

namespace codablecash {

class AbstractJsonObject {
public:
	static const constexpr uint8_t JSON_TYPE_OBJECT = 1;
	static const constexpr uint8_t JSON_TYPE_ARRAY_OBJECT = 2;
	static const constexpr uint8_t JSON_TYPE_VALUE_PAIR = 3;
	static const constexpr uint8_t JSON_TYPE_STRING_VALUE = 4;
	static const constexpr uint8_t JSON_TYPE_BOOLEAN_VALUE = 5;
	static const constexpr uint8_t JSON_TYPE_NUMERIC_VALUE = 6;

	AbstractJsonObject();
	virtual ~AbstractJsonObject();

	virtual uint8_t getType() const noexcept = 0;
	virtual AbstractJsonObject* copy() const noexcept = 0;
	virtual bool equals(const AbstractJsonObject* other) const noexcept = 0;

	virtual int binarySize() const = 0;
	virtual void toBinary(ByteBuffer *out) const = 0;
	virtual void fromBinary(ByteBuffer *in) = 0;
	static AbstractJsonObject* createFromBinary(ByteBuffer* in);
};

} /* namespace codablecash */

#endif /* JSON_OBJECT_ABSTRACTJSONOBJECT_H_ */
