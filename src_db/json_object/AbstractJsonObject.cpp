/*
 * AbstractJsonObject.cpp
 *
 *  Created on: Sep 26, 2025
 *      Author: iizuka
 */

#include "json_object/AbstractJsonObject.h"
#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonArrayObject.h"

#include "base_io/ByteBuffer.h"

#include "json_object/JsonStringValue.h"

#include "json_object/JsonBooleanValue.h"

#include "json_object/JsonNumericValue.h"
namespace codablecash {

AbstractJsonObject::AbstractJsonObject() {

}

AbstractJsonObject::~AbstractJsonObject() {

}

AbstractJsonObject* AbstractJsonObject::createFromBinary(ByteBuffer *in) {
	AbstractJsonObject* object = nullptr;

	uint8_t type = in->get();
	switch(type){
	case JSON_TYPE_OBJECT:
		object = new JsonObject();
		break;
	case JSON_TYPE_ARRAY_OBJECT:
		object = new JsonArrayObject();
		break;
	case JSON_TYPE_VALUE_PAIR:
		object = new JsonValuePair();
		break;
	case JSON_TYPE_STRING_VALUE:
		object = new JsonStringValue();
		break;
	case JSON_TYPE_BOOLEAN_VALUE:
		object = new JsonBooleanValue();
		break;
	case JSON_TYPE_NUMERIC_VALUE:
		object = new JsonNumericValue();
		break;
	default:
		return nullptr;
	}

	object->fromBinary(in);

	return object;
}

} /* namespace codablecash */
