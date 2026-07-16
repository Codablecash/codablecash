/*
 * ScSignature.cpp
 *
 *  Created on: 2022/03/23
 *      Author: iizuka
 */

#include "ScSignature.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"

#include "crypto/Sha256.h"

namespace codablecash {

ScSignature::ScSignature(const ScSignature& inst): e(inst.e), y(inst.y) {
}

ScSignature::ScSignature() : e((int64_t)0), y((int64_t)0){

}

ScSignature::~ScSignature() {

}

void ScSignature::sign(ByteBuffer* data, const BigInteger s) {
	Secp256k1Point G;

	BigInteger r = BigInteger::random().mod(Secp256k1Point::p);

	Secp256k1Point rG = G.multiple(r);

	ByteBuffer* rGArray = rG.to65Bytes(); __STP(rGArray);
	data->position(0);

	int length = data->limit() + rGArray->limit();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(length, true); __STP(buff);
	buff->put(data);
	buff->put(rGArray);

	ByteBuffer* hashed = hash(buff); __STP(hashed);

	BigInteger* e = BigInteger::fromBinary((const char*)hashed->array(), hashed->limit()); __STP(e);
	this->e = e->mod(Secp256k1Point::p);
	this->y = r.subtract(s.multiply(this->e)).mod(Secp256k1Point::n);
}

ByteBuffer* ScSignature::hash(ByteBuffer* buff) {
	buff->position(0);

	return Sha256::sha256((const char*)buff->array(), buff->limit(), true);
}

bool ScSignature::verify(ByteBuffer* data, const Secp256k1Point& pubKey) {
	Secp256k1Point G;

	Secp256k1Point yG = G.multiple(this->y);
	Secp256k1Point eP = pubKey.multiple(this->e);

	Secp256k1Point rG = yG.add(eP);

	ByteBuffer* ptArray = rG.to65Bytes(); __STP(ptArray);
	data->position(0);

	int length = data->limit() + ptArray->limit();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(length, true); __STP(buff);
	buff->put(data);
	buff->put(ptArray);

	ByteBuffer* hashed = hash(buff); __STP(hashed);
	BigInteger* e2ptr = BigInteger::fromBinary((const char*)hashed->array(), hashed->limit()); __STP(e2ptr);
	BigInteger e2 = e2ptr->mod(Secp256k1Point::p);

	int cmp = e2.compareTo(this->e);

	return cmp == 0;
}


} /* namespace codablecash */
