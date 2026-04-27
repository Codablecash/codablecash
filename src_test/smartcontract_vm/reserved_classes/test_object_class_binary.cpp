/*
 * test_object_class_binary.cpp
 *
 *  Created on: Nov 7, 2025
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"

#include "../VmTestUtils.h"
#include "ext_binary/ExtClassObject.h"

#include "instance/reserved_classes/object/ObjectObjectMethod.h"
#include "instance/reserved_classes/object/ObjectClassDeclare.h"

using namespace alinous;

TEST_GROUP(TestObjectClassBinaryGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};

TEST(TestObjectClassBinaryGroup, case01){
	ObjectObjectMethod* method = new ObjectObjectMethod(); __STP(method);

	int cap = method->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	method->toBinary(buff);

	buff->position(0);

	CodeElement* element = CodeElement::createFromBinary(buff); __STP(element);
	ObjectObjectMethod* method2 = dynamic_cast<ObjectObjectMethod*>(element);
	CHECK(method2 != nullptr);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	method2->toBinary(buff2);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

TEST(TestObjectClassBinaryGroup, case02){
	ObjectClassDeclare* clazz = new ObjectClassDeclare(); __STP(clazz);

	int cap = clazz->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true);  __STP(buff);
	clazz->toBinary(buff);

	buff->position(0);

	CodeElement* element = CodeElement::createFromBinary(buff); __STP(element);
	ObjectClassDeclare* clazz2 = dynamic_cast<ObjectClassDeclare*>(element);
	CHECK(clazz2 != nullptr);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true);  __STP(buff2);
	clazz2->toBinary(buff2);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

