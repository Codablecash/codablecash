/*
 * MuSig.h
 *
 *  Created on: 2023/02/02
 *      Author: iizuka
 */

#ifndef MUSIG_MUSIG_H_
#define MUSIG_MUSIG_H_

#include "ecda/Secp256k1Point.h"
#include "numeric/BigInteger.h"

#include "base/ArrayList.h"

namespace codablecash {

class MuSig {
public:
	MuSig &operator=(const MuSig &inst);

	MuSig(const MuSig& inst);
	MuSig(const Secp256k1Point R, const BigInteger s);
	virtual ~MuSig();

	void addXi(const Secp256k1Point Xi) noexcept;

	bool verify(const char *data, int length) const noexcept;

	const Secp256k1Point* getR() const noexcept{
		return &this->R;
	}
	const BigInteger* gets() const noexcept {
		return &this->s;
	}

	UnicodeString* toString() const;

private:
	Secp256k1Point calcX() const noexcept;
	BigInteger calcL() const noexcept;
private:
	Secp256k1Point R;
	BigInteger s;

	ArrayList<Secp256k1Point>* XiList;
};

} /* namespace codablecash */

#endif /* MUSIG_MUSIG_H_ */
