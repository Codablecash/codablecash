/*
 * test_json_copy.cpp
 *
 *  Created on: Oct 16, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "json/JsonHandler.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonArrayObject.h"

#include "json_object/JsonBooleanValue.h"
#include "json_object/JsonStringValue.h"
#include "json_object/JsonNumericValue.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"
using namespace codablecash;


TEST_GROUP(TestJsonCopyGroup) {
	TEST_SETUP() {
		this->env->setup();
	}
	TEST_TEARDOWN() {
		this->env->teardown();
	}
};

TEST(TestJsonCopyGroup, case01) {
	JsonBooleanValue bl;
	JsonBooleanValue* bl2 = dynamic_cast<JsonBooleanValue*>(bl.copy()); __STP(bl2);

	bool eq = bl.equals(bl2);
	CHECK(eq);

	int cap = bl2->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	bl2->toBinary(buff);

	buff->position(0);
	AbstractJsonObject* obj = AbstractJsonObject::createFromBinary(buff); __STP(obj);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	obj->toBinary(buff2);
	buff2->position(0);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

TEST(TestJsonCopyGroup, case02) {
	JsonStringValue value(L"test");
	JsonStringValue* value2 = dynamic_cast<JsonStringValue*>(value.copy()); __STP(value2);

	bool eq = value.equals(value2);
	CHECK(eq);

	int cap = value2->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	value2->toBinary(buff);

	buff->position(0);
	AbstractJsonObject* obj = AbstractJsonObject::createFromBinary(buff); __STP(obj);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	obj->toBinary(buff2);
	buff2->position(0);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

TEST(TestJsonCopyGroup, case03) {
	JsonNumericValue value(10);
	JsonNumericValue* value2 = dynamic_cast<JsonNumericValue*>(value.copy()); __STP(value2);

	bool eq = value.equals(value2);
	CHECK(eq);

	int cap = value2->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	value2->toBinary(buff);

	buff->position(0);
	AbstractJsonObject* obj = AbstractJsonObject::createFromBinary(buff); __STP(obj);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	obj->toBinary(buff2);
	buff2->position(0);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

TEST(TestJsonCopyGroup, case04) {
	JsonValuePair value;
	value.setKey(new JsonStringValue(L"test"));
	value.setValue(new JsonStringValue(L"value"));

	JsonValuePair* value2 = dynamic_cast<JsonValuePair*>(value.copy()); __STP(value2);

	bool eq = value.equals(value2);
	CHECK(eq);

	int cap = value2->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	value2->toBinary(buff);

	buff->position(0);
	AbstractJsonObject* obj = AbstractJsonObject::createFromBinary(buff); __STP(obj);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	obj->toBinary(buff2);
	buff2->position(0);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}


TEST(TestJsonCopyGroup, case05) {
	const File* projectFolder = this->env->getProjectRoot();
	File* file = projectFolder->get(L"src_test/db/db_json/resources02/config.json"); __STP(file);

	JsonHandler handler;
	handler.loadFile(file, 4);

	AbstractJsonObject* object = handler.parse(); __STP(object);
	JsonObject* value = dynamic_cast<JsonObject*>(object);

	JsonObject* value2 = dynamic_cast<JsonObject*>(value->copy()); __STP(value2);

	bool eq = value->equals(value2);
	CHECK(eq);

	int cap = value2->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	value2->toBinary(buff);

	buff->position(0);
	AbstractJsonObject* obj = AbstractJsonObject::createFromBinary(buff); __STP(obj);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	obj->toBinary(buff2);
	buff2->position(0);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

TEST(TestJsonCopyGroup, case06){
	const File* projectFolder = this->env->getProjectRoot();
	File* file = projectFolder->get(L"src_test/db/db_json/resources02/array01.json"); __STP(file);

	JsonHandler handler;
	handler.loadFile(file, 4);

	AbstractJsonObject* object = handler.parse(); __STP(object);
	JsonObject* value = dynamic_cast<JsonObject*>(object);

	JsonObject* value2 = dynamic_cast<JsonObject*>(value->copy()); __STP(value2);

	bool eq = value->equals(value2);
	CHECK(eq);

	int cap = value2->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	value2->toBinary(buff);

	buff->position(0);
	AbstractJsonObject* obj = AbstractJsonObject::createFromBinary(buff); __STP(obj);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	obj->toBinary(buff2);
	buff2->position(0);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

TEST(TestJsonCopyGroup, case07){
	const File* projectFolder = this->env->getProjectRoot();
	File* file = projectFolder->get(L"src_test/db/db_json/resources02/array01.json"); __STP(file);

	JsonHandler handler;
	handler.loadFile(file, 4);

	AbstractJsonObject* object = handler.parse(); __STP(object);

	File* file2 = projectFolder->get(L"src_test/db/db_json/resources02/array02.json"); __STP(file2);
	JsonHandler handler2;
	handler2.loadFile(file2, 4);

	AbstractJsonObject* object2 = handler2.parse(); __STP(object2);

	bool bl = object->equals(object2);
	CHECK(!bl);

	int cap = object2->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	object2->toBinary(buff);

	buff->position(0);
	AbstractJsonObject* obj = AbstractJsonObject::createFromBinary(buff); __STP(obj);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	obj->toBinary(buff2);
	buff2->position(0);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

TEST(TestJsonCopyGroup, case08){
	const File* projectFolder = this->env->getProjectRoot();
	File* file = projectFolder->get(L"src_test/db/db_json/resources02/json01.json"); __STP(file);

	JsonHandler handler;
	handler.loadFile(file, 4);

	AbstractJsonObject* object = handler.parse(); __STP(object);

	File* file2 = projectFolder->get(L"src_test/db/db_json/resources02/json02.json"); __STP(file2);
	JsonHandler handler2;
	handler2.loadFile(file2, 4);

	AbstractJsonObject* object2 = handler2.parse(); __STP(object2);

	bool bl = object->equals(object2);
	CHECK(!bl);

	int cap = object2->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	object2->toBinary(buff);

	buff->position(0);
	AbstractJsonObject* obj = AbstractJsonObject::createFromBinary(buff); __STP(obj);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	obj->toBinary(buff2);
	buff2->position(0);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}
