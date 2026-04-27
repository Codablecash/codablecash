/*
 * test_generic_list_binary.cpp
 *
 *  Created on: Nov 7, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../VmTestUtils.h"
#include "ext_binary/ExtClassObject.h"

#include "instance/reserved_classes/list/ListListMethod.h"
#include "instance/reserved_classes/list/ListClassDeclare.h"

using namespace alinous;

TEST_GROUP(TestGenericListBinaryGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};


TEST(TestGenericListBinaryGroup, case01) {
	ListListMethod* method = new ListListMethod(); __STP(method);

	int cap = method->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	method->toBinary(buff);

	buff->position(0);

	CodeElement* element = CodeElement::createFromBinary(buff); __STP(element);
	ListListMethod* method2 = dynamic_cast<ListListMethod*>(element);
	CHECK(method2 != nullptr);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true); __STP(buff2);
	method2->toBinary(buff2);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

TEST(TestGenericListBinaryGroup, case02) {
	ListClassDeclare* clazz = new ListClassDeclare(); __STP(clazz);

	int cap = clazz->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true);  __STP(buff);
	clazz->toBinary(buff);

	buff->position(0);

	CodeElement* element = CodeElement::createFromBinary(buff); __STP(element);
	ListClassDeclare* clazz2 = dynamic_cast<ListClassDeclare*>(element);
	CHECK(clazz2 != nullptr);

	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap, true);  __STP(buff2);
	clazz2->toBinary(buff2);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}

