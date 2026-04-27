/*
 * JsonValuePair.h
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#ifndef JSON_OBJECT_JSONVALUEPAIR_H_
#define JSON_OBJECT_JSONVALUEPAIR_H_

#include "json_object/AbstractJsonObject.h"


namespace codablecash {

class AbstractJsonValue;

class JsonValuePair : public AbstractJsonObject {
public:
	JsonValuePair(const JsonValuePair& inst);
	JsonValuePair();
	virtual ~JsonValuePair();

	virtual uint8_t getType() const noexcept {
		return JSON_TYPE_VALUE_PAIR;
	}

	void setKey(AbstractJsonValue* key) noexcept;
	void setValue(AbstractJsonObject* value) noexcept;

	AbstractJsonValue* getKey() const noexcept {
		return this->key;
	}
	AbstractJsonObject* getValue() const noexcept {
		return this->value;
	}

	virtual AbstractJsonObject* copy() const noexcept;
	virtual bool equals(const AbstractJsonObject* other) const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer *out) const;
	virtual void fromBinary(ByteBuffer *in);

private:
	AbstractJsonValue* key;
	AbstractJsonObject* value;
};

} /* namespace codablecash */

#endif /* JSON_OBJECT_JSONVALUEPAIR_H_ */
