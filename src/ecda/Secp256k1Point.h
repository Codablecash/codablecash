/*
 * Secp256k1Point.h
 *
 *  Created on: 2022/03/23
 *      Author: iizuka
 */

#ifndef ECDA_SECP256K1POINT_H_
#define ECDA_SECP256K1POINT_H_

#include "numeric/BigInteger.h"

#include "ecda/Secp256k1CompressedPoint.h"
using namespace alinous;

namespace codablecash {

class Secp256k1Point {
public:
	/**
	 * 32 bytes value
	 *
	 * FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFC2F
	 */
	static const BigInteger p; // = new BigInteger("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", 16);

	/**
	 * y^2 = x^3 + 7 mod p
	 */
	static const BigInteger a; // = new BigInteger("0", 16);
	static const BigInteger b; // = new BigInteger("7", 16);

	/**
	 * x: 0x79BE667E F9DCBBAC 55A06295 CE870B07 029BFCDB 2DCE28D9 59F2815B 16F81798
	 * y: 0x483ADA77 26A3C465 5DA4FBFC 0E1108A8 FD17B448 A6855419 9C47D08F FB10D4B8
	 */
	static const BigInteger gX; //  = new BigInteger("79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798", 16);
	static const BigInteger gY; //  = new BigInteger("483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8", 16);

	// n * G = O(zero)
	static const BigInteger n; //  = new BigInteger ("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141", 16);


	static const BigInteger Zero; //= new BigInteger("0");
	static const BigInteger One; // = new BigInteger("1");
	static const BigInteger Two; // = new BigInteger("2");
	static const BigInteger Three; // = new BigInteger("3");

	Secp256k1Point &operator=(const Secp256k1Point &inst);

	Secp256k1Point(const Secp256k1Point& inst);
	Secp256k1Point();
	Secp256k1Point(const BigInteger x, const BigInteger y);

	virtual ~Secp256k1Point();

	//Secp256k1Point negate();
	Secp256k1Point multiple(const BigInteger& value) const;
	Secp256k1Point multiple(int val) const;

	Secp256k1Point add(const Secp256k1Point& pt);

	bool isO() const noexcept;

	bool equals(const Secp256k1Point& pt) const noexcept;

	const BigInteger& getX() const noexcept {
		return this->x;
	}
	const BigInteger& getY() const noexcept {
		return this->y;
	}

	ByteBuffer* to65Bytes() const;
	static Secp256k1Point from65Bytes(ByteBuffer* buff);

	UnicodeString* toString(int radix) const noexcept;

private:
	Secp256k1Point doMultiple(const BigInteger& val) const;
	BigInteger getRambda(const Secp256k1Point& pt);

protected:
	BigInteger x;
	BigInteger y;
};

} /* namespace codablecash */

#endif /* ECDA_SECP256K1POINT_H_ */
