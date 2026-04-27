/*
 * SimpleMuSigSigner.cpp
 *
 *  Created on: 2023/02/02
 *      Author: iizuka
 */

#include "musig/SimpleMuSigSigner.h"

#include "ecda/Secp256k1Point.h"

#include "musig/MuSigHashBuilder.h"
namespace codablecash {

SimpleMuSigSigner::SimpleMuSigSigner(const BigInteger x) : x(x), r(0L) {

}

SimpleMuSigSigner::~SimpleMuSigSigner() {

}

Secp256k1Point SimpleMuSigSigner::getxG() {
	Secp256k1Point G;
	Secp256k1Point xG = G.multiple(this->x);
	return xG;
}

Secp256k1Point SimpleMuSigSigner::getrG() {
	this->r = BigInteger::ramdom(BigInteger(0L), Secp256k1Point::n);
	Secp256k1Point G;

	return G.multiple(this->r);
}

/**
 * Each signer computes si = ri + H(X,R,m)H(L,Xi)xi
 * HXRm: H(X,R,m)
 */
BigInteger SimpleMuSigSigner::gets(const BigInteger *HXRm, const BigInteger *L) {
	Secp256k1Point Xi = getxG();

	BigInteger HLXi(0L);
	{
		MuSigHashBuilder hashBuilder;
		hashBuilder.addBigInteger(L);
		hashBuilder.add(&Xi);
		hashBuilder.buildHash();

		HLXi = hashBuilder.getResultAsBigInteger();
	}

	BigInteger v2 = HXRm->multiply(HLXi).multiply(this->x);
	BigInteger ret = this->r.add(v2);

	return ret;
}

} /* namespace codablecash */
