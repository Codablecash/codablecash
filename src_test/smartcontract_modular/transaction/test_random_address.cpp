/*
 * test_random_address.cpp
 *
 *  Created on: Mar 17, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "bc_wallet/HdWalletSeed.h"

#include "numeric/BigInteger.h"

#include "ecda/ScPrivateKey.h"
#include "ecda/ScPublicKey.h"

#include "transaction/SmartcontractModuleAddress.h"


using namespace codablecash;

TEST_GROUP(TestRandomAddressGroup) {
	TEST_SETUP() {
	}
	TEST_TEARDOWN() {
	}
};

TEST(TestRandomAddressGroup, case01){
	SmartcontractModuleAddress* address = nullptr;
	SmartcontractModuleAddress* address2 = nullptr;
	{
		HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
		BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
		ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);

		ScPublicKey publicKey = privateKey->generatePublicKey();
		address = SmartcontractModuleAddress::createAddress(1, &publicKey);
	}
	__STP(address);
	{
		HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
		BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
		ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);

		ScPublicKey publicKey = privateKey->generatePublicKey();
		address2 = SmartcontractModuleAddress::createAddress(1, &publicKey);
	}
	__STP(address2);


	int cmp = address2->compareTo(address);
	CHECK(cmp != 0);
}


TEST(TestRandomAddressGroup, case02){
	BigInteger seed = BigInteger::ramdom();
	BigInteger seed2 = BigInteger::ramdom();

	bool bl = seed.equals(&seed2);
	CHECK(!bl);
}

TEST(TestRandomAddressGroup, case03){
	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);

	HdWalletSeed* seed2 = HdWalletSeed::newSeed(); __STP(seed2);
	BigInteger* bigInt2 = seed2->toBigInteger(); __STP(bigInt2);

	int cmp = bigInt2->compareTo(*bigInt);
	CHECK(cmp != 0);
}

TEST(TestRandomAddressGroup, case04){
	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
	ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);
	ScPublicKey publicKey = privateKey->generatePublicKey();

	HdWalletSeed* seed2 = HdWalletSeed::newSeed(); __STP(seed2);
	BigInteger* bigInt2 = seed2->toBigInteger(); __STP(bigInt2);
	ScPrivateKey* privateKey2 = new ScPrivateKey(bigInt2, 1); __STP(privateKey2);
	ScPublicKey publicKey2 = privateKey2->generatePublicKey();

	int cmp = bigInt2->compareTo(*bigInt);
	CHECK(cmp != 0);

	cmp = privateKey->compareTo(privateKey2);
	CHECK(cmp != 0);

	bool bl = publicKey2.equals(&publicKey);
	CHECK(!bl);
}
