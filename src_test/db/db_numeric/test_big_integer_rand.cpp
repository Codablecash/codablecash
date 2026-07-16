/*
 * test_big_integer_rand.cpp
 *
 *  Created on: Jul 15, 2026
 *      Author: iizuka
 */

#include "base_io/ByteBuffer.h"
#include "test_utils/t_macros.h"

#include "numeric/BigInteger.h"
#include "base/UnicodeString.h"
#include "base/StackRelease.h"

using namespace alinous;

TEST_GROUP(TestBigIntegerRandGroup) {
	TEST_SETUP() {}
	TEST_TEARDOWN() {}
};

TEST(TestBigIntegerRandGroup, nano){
	uint64_t nano = Os::getNanoSec();
	uint64_t nano2 = Os::getNanoSec();
}


TEST(TestBigIntegerRandGroup, nanocat){
	mpz_t s;
	mpz_init(s);

	uint64_t nano = Os::getNanoSec();
	uint32_t n32 = (uint32_t)nano;

	mpz_add_ui(s, s, n32);

	nano = Os::getNanoSec();
	mpz_mul_2exp(s, s, 32);
	mpz_add_ui(s, s, n32);

	mpz_clear(s);
}

TEST(TestBigIntegerRandGroup, nanocat2) {
	mpz_t s;
	mpz_init(s);

	int maxLoop = 256 / 32;
	for(int i = 0; i != maxLoop; ++i){
		uint64_t nano = Os::getNanoSec();
		uint32_t n32 = (uint32_t)nano;

		mpz_mul_2exp(s, s, 32);
		mpz_add_ui(s, s, n32);
	}

	mpz_clear(s);
}

TEST(TestBigIntegerRandGroup, CSPRNG) {
	uint32_t rand = Os::getOsCspring();
	uint32_t rand2 = Os::getOsCspring();
}

TEST(TestBigIntegerRandGroup, rand01) {
	BigInteger bi01 = BigInteger::random();
	BigInteger bi02 = BigInteger::random();
}

