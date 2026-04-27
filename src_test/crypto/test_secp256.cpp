/*
 * test_secp256.cpp
 *
 *  Created on: 2022/03/23
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"
#include "numeric/BigInteger.h"

#include "ecda/Secp256k1Point.h"
#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "../../src/ecda/ScSignature.h"
using namespace codablecash;
using namespace alinous;

TEST_GROUP(TestSecp256Group) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}

};

TEST(TestSecp256Group, checkBasePoint){
	BigInteger gx = Secp256k1Point::gX;

	BigInteger right = gx.pow(3).add(Secp256k1Point::b);
	right = right.mod(Secp256k1Point::p);

	BigInteger left = Secp256k1Point::gY.pow(2).mod(Secp256k1Point::p);

	bool bl = left.equals(&right);
	CHECK(bl == true);
}

TEST(TestSecp256Group, checkOrder){
	Secp256k1Point pt;
	Secp256k1Point Opt(BigInteger(L"0", 10), BigInteger(L"0", 10));

	BigInteger L = Secp256k1Point::n;
	Secp256k1Point mul = pt.multiple(L);

	bool bl = Opt.equals(mul);
	CHECK(bl == true);
}

TEST(TestSecp256Group, testDouble){
	Secp256k1Point pt;

	Secp256k1Point pt2 = pt.add(pt);
	Secp256k1Point pt4 = pt2.add(pt2);

	{
		UnicodeString* str = pt4.toString(16);
		delete str;
	}


	Secp256k1Point ptadd4 = pt.add(pt).add(pt).add(pt);

	bool bl = pt4.equals(ptadd4);
	CHECK(bl == true);
}

TEST(TestSecp256Group, mulMinus){
	Secp256k1Point pt;
	Secp256k1Point pa = pt.multiple(BigInteger(L"-1", 10));
	Secp256k1Point pb = pt.multiple(BigInteger(L"2", 10));

	Secp256k1Point ans = pa.add(pb);

	Secp256k1Point pt4 = pt;

	bool bl = pt4.equals(ans);
	CHECK(bl == true);
}

TEST(TestSecp256Group, multiplySimple){
	int n = 30;

	Secp256k1Point pt;

	Secp256k1Point mul = pt.multiple(n);

	Secp256k1Point ans(Secp256k1Point::Zero, Secp256k1Point::Zero);
	for(int i = 0; i != n; ++i){
		ans = ans.add(pt);
	}

	bool bl = mul.equals(ans);
	CHECK(bl == true);
}

TEST(TestSecp256Group, addInversePoint){
	Secp256k1Point pt(Secp256k1Point::gX, Secp256k1Point::gY);
	Secp256k1Point pt2(Secp256k1Point::gX, Secp256k1Point::gY.negate());

	Secp256k1Point ans = pt.add(pt2);

	bool bl = ans.isO();
	CHECK(bl == true);
}

TEST(TestSecp256Group, zero){
	Secp256k1Point pt;

	Secp256k1Point add0 = pt.add(pt.multiple(0));
	bool bl = add0.equals(pt);
	CHECK(bl == true);

	Secp256k1Point add02 = pt.add(pt.multiple(BigInteger((int64_t)0)));
	bl = add02.equals(pt);
	CHECK(bl == true);
}

TEST(TestSecp256Group, binary){
	Secp256k1Point pt(Secp256k1Point::gX, Secp256k1Point::gY);

	ByteBuffer* buff = pt.to65Bytes(); __STP(buff);

	buff->position(0);
	Secp256k1Point pt2 = Secp256k1Point::from65Bytes(buff);

	BigInteger x = pt.getX();
	BigInteger x2 = pt2.getX();
	bool bl = x2.equals(&x);
	CHECK(bl == true);

	bl = pt2.equals(pt);
	CHECK(bl == true);
}

TEST(TestSecp256Group, mul01){
	BigInteger r = BigInteger::ramdom().mod(Secp256k1Point::p);
	Secp256k1Point pt;

	Secp256k1Point rG = pt.multiple(r);

	int64_t v = rG.getX().longValue();

	ByteBuffer* rGArray = rG.to65Bytes(); __STP(rGArray);
	int size = rGArray->limit();

	CHECK(size == 65);
}


