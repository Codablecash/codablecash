/*
 * test_pow_random_hash.cpp
 *
 *  Created on: 2023/03/09
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "pow_hash/PowRandomHashManager.h"
#include "pow_hash/RandomShaker.h"
#include "pow_hash/TotalNumberSplitter.h"

#include "numeric/BigInteger.h"

#include "ecda/Secp256k1Point.h"


using namespace codablecash;

TEST_GROUP(TestPoWRandomHashGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};

TEST(TestPoWRandomHashGroup, case01){
	BigInteger blockHash = BigInteger::random(BigInteger(0L), BigInteger(Secp256k1Point::p));
	BigInteger nonce = BigInteger::random(BigInteger(0L), BigInteger(Secp256k1Point::p));
	PowRandomHashManager manager;

	ByteBuffer* buff = manager.calculate(&blockHash, &nonce); __STP(buff);

}

TEST(TestPoWRandomHashGroup, shaker01){
	BigInteger nonce = BigInteger::random(BigInteger(0L), BigInteger(Secp256k1Point::p));
	RandomShaker shaker(&nonce, 4);

}

TEST(TestPoWRandomHashGroup, TotalNumberSplitter01){
	BigInteger nonce = BigInteger::random(BigInteger(0L), BigInteger(Secp256k1Point::p));

	int num = 4;
	TotalNumberSplitter splitter(&nonce, num, 200);

	int total = 0;
	for(int i = 0; i != num; ++i){
		int num = splitter.get(i);
		total += num;
	}

	CHECK(total == 200);
}

