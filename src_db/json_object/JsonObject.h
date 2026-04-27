/*
 * JsonObject.h
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#ifndef JSON_OBJECT_JSONOBJECT_H_
#define JSON_OBJECT_JSONOBJECT_H_

#include "json_object/AbstractJsonContainer.h"

#include "base/ArrayList.h"

using namespace alinous;

namespace codablecash {

class JsonValuePair;

class JsonObject : public AbstractJsonContainer {
public:
	JsonObject(const JsonObject& inst);
	JsonObject();
	virtual ~JsonObject();

	virtual uint8_t getType() const noexcept {
		return JSON_TYPE_OBJECT;
	}

	void add(JsonValuePair* pair) noexcept;

	const JsonValuePair* get(int pos) const noexcept;
	const JsonValuePair* get(const wchar_t* name) const noexcept;
	int size() const noexcept;
	bool hasNamedPair(const wchar_t* name) const noexcept;

	virtual AbstractJsonObject* copy() const noexcept;
	virtual bool equals(const AbstractJsonObject* other) const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer *out) const;
	virtual void fromBinary(ByteBuffer *in);

private:
	ArrayList<JsonValuePair>* list;
};

} /* namespace codablecash */

#endif /* JSON_OBJECT_JSONOBJECT_H_ */
