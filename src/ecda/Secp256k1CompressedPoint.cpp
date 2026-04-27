/*
 * Secp256k1CompressedPoint.cpp
 *
 *  Created on: 2023/01/31
 *      Author: iizuka
 */

#include "ecda/Secp256k1CompressedPoint.h"
#include "ecda/Secp256k1Point.h"

#include <cassert>

#include "ecda/Secp256k1Exception.h"

#include "base/StackRelease.h"

#include "numeric/BigInteger.h"
namespace codablecash {

Secp256k1CompressedPoint& Secp256k1CompressedPoint::operator =(const Secp256k1CompressedPoint &inst) {
	if(this != &inst){
		this->x = inst.x;
		this->prefix = inst.prefix;
	}
	return(*this);
}

Secp256k1CompressedPoint::Secp256k1CompressedPoint(const Secp256k1CompressedPoint& inst) : x(inst.x), prefix(inst.prefix){

}

Secp256k1CompressedPoint::Secp256k1CompressedPoint(const BigInteger x, uint8_t prefix)
		: x(x), prefix(prefix) {
}

Secp256k1CompressedPoint::~Secp256k1CompressedPoint() {

}



Secp256k1CompressedPoint Secp256k1CompressedPoint::compress(const Secp256k1Point *pt) {
	const BigInteger x = pt->getX();
	const BigInteger y = pt->getY();

	int64_t MOD = y.mod(BigInteger::TWO).longValue();
	uint8_t prefix = MOD == 0 ? Secp256k1CompressedPoint::COMPRESS_Y_EVEN : Secp256k1CompressedPoint::COMPRESS_Y_ODD;

	return Secp256k1CompressedPoint(x, prefix);
}

/**
 * def decompress_pubkey(pk):
    x = int.from_bytes(pk[1:33], byteorder='big')
    y_sq = (pow(x, 3, p) + 7) % p
    y = pow(y_sq, (p + 1) // 4, p)
    if y % 2 != pk[0] % 2:
        y = p - y
    y = y.to_bytes(32, byteorder='big')
    return b'\x04' + pk[1:33] + y
 */

Secp256k1Point Secp256k1CompressedPoint::decompress() const {
	BigInteger three(3L), seven(7L), four(4L);
	BigInteger y_sq = this->x.modPow(three, Secp256k1Point::p).add(seven)
			.mod(Secp256k1Point::p);

	//if(!y_sq.equals(&BigInteger::ZERO)){
	if(!BigInteger::isSqrt(y_sq, Secp256k1Point::p)){
		throw new Secp256k1Exception(L"The point is not on Elliptic Curve defined by Secp256k.", __FILE__, __LINE__);
	}

	BigInteger P_1 = Secp256k1Point::p.add(BigInteger::ONE).shiftRight(2);
	BigInteger y = y_sq.modPow(P_1, Secp256k1Point::p);

	int64_t ymod = y.mod(BigInteger::TWO).longValue();

	if(ymod != (this->prefix % 2)){
		y = Secp256k1Point::p.subtract(y).mod(Secp256k1Point::p);
	}

	return Secp256k1Point(this->x, y);
	/*
	// y^{2} = x^{3} + 7
	BigInteger three(3L), seven(7L);
	BigInteger tmp = this->x.modPow(three, Secp256k1Point::p).add(seven)
			.mod(Secp256k1Point::p);

	BigInteger result(0L);
	bool ret = tmp.sqrtm(Secp256k1Point::p, &result);
	if(!ret){
		throw new Secp256k1Exception(L"The point is not on Elliptic Curve defined by Secp256k.", __FILE__, __LINE__);
	}

	if(this->prefix == Secp256k1CompressedPoint::COMPRESS_Y_ODD){
		result = result.negate();
	}

	return Secp256k1Point(this->x, result);*/
}

int Secp256k1CompressedPoint::binarySize() const {
	int total = sizeof(this->prefix);
	total += 32; //this->x.binarySize();

	return total;
}

void Secp256k1CompressedPoint::toBinary(ByteBuffer *out) const {
	out->put(this->prefix);

	ByteBuffer* buffx = this->x.toBinary(); __STP(buffx);
	buffx->position(0);

	ByteBuffer* buffxp = BigInteger::padBuffer(buffx, 32); __STP(buffxp);
	buffxp->position(0);
	out->put(buffxp);
}

Secp256k1CompressedPoint* Secp256k1CompressedPoint::fromBinary(ByteBuffer* in) {
	uint8_t prefix = in->get();

	uint8_t datax[32];
	//Mem::memset(datax, 0, 32);
	in->get(datax, 32);

	BigInteger* xptr = BigInteger::fromBinary((const char*)datax, 32); __STP(xptr);

	return new Secp256k1CompressedPoint(*xptr, prefix);
}

IBlockObject* Secp256k1CompressedPoint::copyData() const noexcept {
	return new Secp256k1CompressedPoint(this->x, this->prefix);
}

int Secp256k1CompressedPoint::compareTo(const Secp256k1CompressedPoint *other) {
	int cmp = this->prefix - other->prefix;
	if(cmp != 0){
		return cmp;
	}

	return this->x.compareTo(other->x);
}

} /* namespace codablecash */
