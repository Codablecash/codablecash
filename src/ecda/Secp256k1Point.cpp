/*
 * Secp256k1Point.cpp
 *
 *  Created on: 2022/03/23
 *      Author: iizuka
 */

#include "Secp256k1Point.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"
namespace codablecash {

const BigInteger Secp256k1Point::p(L"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", 16);

const BigInteger Secp256k1Point::a(L"0", 16);
const BigInteger Secp256k1Point::b(L"7", 16);

const BigInteger Secp256k1Point::gX(L"79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798", 16);
const BigInteger Secp256k1Point::gY(L"483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8", 16);

const BigInteger Secp256k1Point::n(L"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141", 16);


const BigInteger Secp256k1Point::Zero(L"0", 16);
const BigInteger Secp256k1Point::One(L"1", 16);
const BigInteger Secp256k1Point::Two(L"2", 16);
const BigInteger Secp256k1Point::Three(L"3", 16);

Secp256k1Point& Secp256k1Point::operator =(const Secp256k1Point& inst) {
	if(this != &inst){
		this->x = inst.x;
		this->y = inst.y;
	}
	return(*this);
}

Secp256k1Point::Secp256k1Point(const Secp256k1Point& inst) : x(inst.x), y(inst.y){

}


Secp256k1Point::Secp256k1Point() : x(gX), y(gY) {

}

Secp256k1Point::Secp256k1Point(const BigInteger x, const BigInteger y) : x(x), y(y) {
}

Secp256k1Point::~Secp256k1Point() {

}
/*
Secp256k1Point Secp256k1Point::negate() {
	return Secp256k1Point(this->x.negate(), this->y.negate());
}
*/
Secp256k1Point Secp256k1Point::multiple(const BigInteger& value) const {
	BigInteger val = value;
	while(val.compareTo(Zero) < 0){
		val = val.add(n);
	}

	return doMultiple(val);
}

Secp256k1Point Secp256k1Point::multiple(int val) const {
	if(val == 0){
		return Secp256k1Point(Zero, Zero);
	}
	if(val == 1){
		return *this;
	}
	int mod2 = val % 2;

	int dblBase = (val - mod2) / 2;

	Secp256k1Point base = multiple(dblBase);
	base = base.add(base);

	return mod2 == 0 ? base : base.add(*this);
}

Secp256k1Point Secp256k1Point::doMultiple(const BigInteger& val) const {
	if(val.equals(&Zero)){
		return Secp256k1Point(Zero, Zero);
	}
	if(val.equals(&One)){
		return *this;
	}

	BigInteger mod2 = val.mod(Two);
	BigInteger dblBase = val.subtract(mod2).divide(Two);

	Secp256k1Point base = doMultiple(dblBase);
	base = base.add(base);


	return mod2.equals(&Zero) ? base : base.add(*this);
}

Secp256k1Point Secp256k1Point::add(const Secp256k1Point& pt) {
	if(pt.isO()){
		return *this;
	}
	if(isO()){
		return pt;
	}
	if(this->x.equals(&pt.x) && this->y.add(pt.y).mod(p).equals(&Zero)){
		return Secp256k1Point(Zero, Zero);
	}

	BigInteger rambda = getRambda(pt);

	BigInteger x4 = rambda.pow(2).subtract(this->x).subtract(pt.x).mod(p);
	BigInteger y4 = rambda.multiply(this->x.subtract(x4)).subtract(this->y).mod(p);


	return Secp256k1Point(x4, y4);
}

bool Secp256k1Point::equals(const Secp256k1Point& pt) const noexcept {
	return this->x.equals(&pt.x) && this->y.equals(&pt.y);
}

ByteBuffer* Secp256k1Point::to65Bytes() const {
	assert(!this->x.isNegative());
	assert(!this->y.isNegative());

	ByteBuffer* buffx = this->x.toBinary(); __STP(buffx);
	ByteBuffer* buffy = this->y.toBinary(); __STP(buffy);

	ByteBuffer* buffxp = BigInteger::padBuffer(buffx, 32); __STP(buffxp);
	ByteBuffer* buffyp = BigInteger::padBuffer(buffy, 32); __STP(buffyp);

	buffxp->position(0);
	buffyp->position(0);

	int size = 1 + buffxp->limit() + buffyp->limit();

	assert(size == 65);

	ByteBuffer* ret = ByteBuffer::allocateWithEndian(size, true); __STP(ret);
	ret->put(0x04);
	ret->put(buffxp);
	ret->put(buffyp);

	return __STP_MV(ret);
}

Secp256k1Point Secp256k1Point::from65Bytes(ByteBuffer* buff) {
	buff->get(); // skip 0x04

	uint8_t datax[32];
	buff->get(datax, 32);

	uint8_t datay[32];
	buff->get(datay, 32);

	BigInteger* xptr = BigInteger::fromBinary((const char*)datax, 32); __STP(xptr);
	BigInteger* yptr = BigInteger::fromBinary((const char*)datay, 32); __STP(yptr);

	return Secp256k1Point(*xptr, *yptr);
}

BigInteger Secp256k1Point::getRambda(const Secp256k1Point& pt) {
	if(equals(pt)){
		BigInteger numerator = this->x.pow(2).multiply(Three);
		BigInteger denominator = this->y.multiply(Two).modInverse(p);

		return numerator.multiply(denominator).mod(p);
	}

	BigInteger numerator = pt.y.subtract(this->y);
	BigInteger denominator = pt.x.subtract(this->x);

	denominator = denominator.modInverse(p);

	return numerator.multiply(denominator).mod(p);
}

bool Secp256k1Point::isO() const noexcept {
	return x.equals(&Zero) && y.equals(&Zero);
}

UnicodeString* Secp256k1Point::toString(int radix) const noexcept {
	UnicodeString xstr = this->x.toString(radix);
	UnicodeString ystr = this->y.toString(radix);

	UnicodeString* ret = new UnicodeString(L"");
	ret->append(&xstr);
	ret->append(L", ");
	ret->append(&ystr);

	return ret;
}

} /* namespace codablecash */
