/*
 * ScPrivateKey.cpp
 *
 *  Created on: 2022/03/23
 *      Author: iizuka
 */

#include "ScPrivateKey.h"

#include "ScPublicKey.h"
#include "numeric/BigInteger.h"

#include "Secp256k1Point.h"
namespace codablecash {

const BigInteger ScPrivateKey::p(L"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", 16);

ScPrivateKey::ScPrivateKey(const BigInteger* seed, uint64_t solt) : keyvalue((int64_t)0) {
	assert(solt != 0);
	BigInteger pow(solt);

	this->keyvalue = seed->multiply(pow).mod(ScPrivateKey::p);
}

ScPrivateKey::ScPrivateKey() : keyvalue((int64_t)0) {
	this->keyvalue = BigInteger::ramdom().mod(ScPrivateKey::p);
}

ScPrivateKey::~ScPrivateKey() {

}

ScPublicKey ScPrivateKey::generatePublicKey() {
	Secp256k1Point BASE_POINT;

	Secp256k1Point pt = BASE_POINT.multiple(this->keyvalue);

	return ScPublicKey(pt);
}

int ScPrivateKey::compareTo(const ScPrivateKey *other) const {
	return this->keyvalue.compareTo(other->keyvalue);
}

} /* namespace codablecash */
