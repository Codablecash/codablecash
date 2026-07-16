/*
 * test_big_integer_misc.cpp
 *
 *  Created on: 2022/06/06
 *      Author: iizuka
 */

#include "base_io/ByteBuffer.h"
#include "test_utils/t_macros.h"

#include "numeric/BigInteger.h"
#include "base/UnicodeString.h"
#include "base/StackRelease.h"

using namespace alinous;

TEST_GROUP(TestBigIntegerMiscGroup) {
	TEST_SETUP() {}
	TEST_TEARDOWN() {}
};

TEST(TestBigIntegerMiscGroup, rnd02){
	BigInteger inst = BigInteger::random(BigInteger(1000L), BigInteger(100000000000L));
	UnicodeString str = inst.toString("%Zd");

	BigInteger mask(0xFF);
	BigInteger ans = inst._and(mask);
	uint64_t v = ans.longValue();
	CHECK( v <= 0xff);
}

TEST(TestBigIntegerMiscGroup, rnd01){
	BigInteger inst = BigInteger::random();
}

TEST(TestBigIntegerMiscGroup, pad01){
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(1, true); __STP(buff);

	buff->put(10);

	buff->position(0);
	ByteBuffer* padded = BigInteger::padBuffer(buff, 10); __STP(padded);

	int limit = padded->limit();
	CHECK(limit == 10);

}
