/*
 * ScPrivateKey.h
 *
 *  Created on: 2022/03/23
 *      Author: iizuka
 */

#ifndef ECDA_SCPRIVATEKEY_H_
#define ECDA_SCPRIVATEKEY_H_

#include "numeric/BigInteger.h"

#include "Secp256k1Point.h"

using namespace alinous;

namespace codablecash {

class ScPublicKey;

class ScPrivateKey {
public:
	static const BigInteger p; // = new BigInteger("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", 16);

	/**
	 * solt must not be 0
	 * @param seed
	 * @param solt
	 */
	ScPrivateKey(const BigInteger* seed, uint64_t solt);
	ScPrivateKey();
	virtual ~ScPrivateKey();

	ScPublicKey generatePublicKey();

	const BigInteger getKeyvalue() const noexcept {
		return keyvalue;
	}

	int compareTo(const ScPrivateKey* other) const;

private:
	BigInteger keyvalue;

};

} /* namespace codablecash */

#endif /* ECDA_SCPRIVATEKEY_H_ */
