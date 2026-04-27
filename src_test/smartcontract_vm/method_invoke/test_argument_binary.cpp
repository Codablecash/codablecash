/*
 * test_argument_binary.cpp
 *
 *  Created on: Mar 10, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "ext_arguments/AbstractFunctionExtArguments.h"
#include "ext_arguments/BoolArgument.h"
#include "ext_arguments/NullArgument.h"
#include "ext_arguments/NumericArgument.h"
#include "ext_arguments/StringArgument.h"

#include "base_io/ByteBuffer.h"

#include "engine/sc_analyze/AnalyzedType.h"


TEST_GROUP(TestArgumentBinaryGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};


TEST(TestArgumentBinaryGroup, boolarg){
	BoolArgument boolarg(true);

	int cap = boolarg.binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

	boolarg.toBinary(buff);

	buff->position(0);
	AbstractFunctionExtArguments* arg = AbstractFunctionExtArguments::createFromBinary(buff); __STP(arg);

	int cap2 = arg->binarySize();
	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap2, true); __STP(buff2);
	arg->toBinary(buff2);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);

	delete arg->copy();
}

TEST(TestArgumentBinaryGroup, nullarg){
	AnalyzedType at(AnalyzedType::TYPE_DOM);
	NullArgument arg(&at);

	int cap = arg.binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

	arg.toBinary(buff);

	buff->position(0);
	AbstractFunctionExtArguments* arg2 = AbstractFunctionExtArguments::createFromBinary(buff); __STP(arg2);

	int cap2 = arg2->binarySize();
	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap2, true); __STP(buff2);
	arg2->toBinary(buff2);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);

	delete arg2->copy();
}

TEST(TestArgumentBinaryGroup, numericarg){
	NumericArgument arg(100, AnalyzedType::TYPE_INT);

	int cap = arg.binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

	arg.toBinary(buff);

	buff->position(0);
	AbstractFunctionExtArguments* arg2 = AbstractFunctionExtArguments::createFromBinary(buff); __STP(arg2);

	int cap2 = arg2->binarySize();
	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap2, true); __STP(buff2);
	arg2->toBinary(buff2);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);

	delete arg2->copy();
}

TEST(TestArgumentBinaryGroup, stringarg){
	UnicodeString test(L"test");
	StringArgument arg(&test);

	int cap = arg.binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

	arg.toBinary(buff);

	buff->position(0);
	AbstractFunctionExtArguments* arg2 = AbstractFunctionExtArguments::createFromBinary(buff); __STP(arg2);

	int cap2 = arg2->binarySize();
	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap2, true); __STP(buff2);
	arg2->toBinary(buff2);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);

	delete arg2->copy();
}

TEST(TestArgumentBinaryGroup, misc01){
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(10, true); __STP(buff);
	buff->put(128);
	buff->position(0);

	AbstractFunctionExtArguments* ret = AbstractFunctionExtArguments::createFromBinary(buff);
	CHECK(ret == nullptr);
}

