/*
 * MuSigBuilder.cpp
 *	Xi = xiG
 * Call L = H(X1,X2,…)
 * Call X the sum of all H(L,Xi)Xi
 * Each signer chooses a random nonce ri, and shares Ri = riG with the other signers
 * Call R the sum of the Ri points
 * Each signer computes si = ri + H(X,R,m)H(L,Xi)xi
 * The final signature is (R,s) where s is the sum of the si values
 * Verification again satisfies sG = R + H(X,R,m)X
 *
 * Proof
 * 	Multiply G (point) and (mod n). n | nG = O
 *	si * G = ri + H(X,R,m)H(L,Xi)xi * G
 *
 *	<--- sG = sum( siG )
 *	<--- R = sum( riG ) = sum( Ri )
 *	<--- X = sum( H(L,Xi)xiG ) = sum(H(L,Xi)Xi )

 *	sG = R + H(X,R,m)X
 *
 *  Created on: 2023/02/01
 *      Author: iizuka
 */

#include "musig/MuSigBuilder.h"
#include "musig/IMuSigSigner.h"
#include "musig/MuSigHashBuilder.h"
#include "musig/MuSig.h"


namespace codablecash {

MuSigBuilder::MuSigBuilder() : L(0L), s(0L) {
	this->signers = new ArrayList<IMuSigSigner>();
	this->XiList = new ArrayList<Secp256k1Point>();
}

MuSigBuilder::~MuSigBuilder() {
	this->signers->deleteElements();
	delete this->signers;

	this->XiList->deleteElements();
	delete this->XiList;
}

void MuSigBuilder::addSigner(IMuSigSigner *signer) noexcept {
	this->signers->addElement(signer);
}

MuSig MuSigBuilder::sign(const char *data, int length) {
	// Xi = xG
	// Call L = H(X1,X2,…)
	calcL();

	// Call X the sum of all H(L,Xi)Xi
	calcX();

	// Each signer chooses a random nonce ri, and shares Ri = riG with the other signers
	// Call R the sum of the Ri points
	calcR();

	// Each signer computes si = ri + H(X,R,m)H(L,Xi)xi
	// The final signature is (R,s) where s is the sum of the si values
	calcs(data, length);

	return MuSig(this->R, this->s);
}

void MuSigBuilder::calcL() {
	MuSigHashBuilder hashBuilder;

	int maxLoop = this->signers->size();
	for(int i = 0; i != maxLoop; ++i){
		IMuSigSigner* signer = this->signers->get(i);

		Secp256k1Point Xi = signer->getxG();
		hashBuilder.add(&Xi);
		this->XiList->addElement(new Secp256k1Point(Xi));
	}
	hashBuilder.buildHash();

	this->L = hashBuilder.getResultAsBigInteger();
	//this->L.modSelf(Secp256k1Point::n);
}

// Call X the sum of all H(L,Xi)Xi
void MuSigBuilder::calcX() {
	Secp256k1Point result(BigInteger(L"0", 10), BigInteger(L"0", 10)); // O point

	int maxLoop = this->XiList->size();
	for(int i = 0; i != maxLoop; ++i){
		Secp256k1Point* Xi = this->XiList->get(i);

		MuSigHashBuilder hashBuilder;
		hashBuilder.addBigInteger(&this->L);
		hashBuilder.add(Xi);
		hashBuilder.buildHash();

		BigInteger hash = hashBuilder.getResultAsBigInteger(); //.mod(Secp256k1Point::n);
		Secp256k1Point value = Xi->multiple(hash);

		result = result.add(value);
	}

	this->X = result;
}

// Each signer chooses a random nonce ri, and shares Ri = riG with the other signers
// Call R the sum of the Ri points
void MuSigBuilder::calcR() {
	Secp256k1Point result(BigInteger(0L), BigInteger(0L)); // O point

	int maxLoop = this->signers->size();
	for(int i = 0; i != maxLoop; ++i){
		IMuSigSigner* signer = this->signers->get(i);

		Secp256k1Point Ri = signer->getrG();
		result = result.add(Ri);
	}

	this->R = result;
}

/*
 * 	 Each signer computes si = ri + H(X,R,m)H(L,Xi)xi
 *	 The final signature is (R,s) where s is the sum of the si values
 */
void MuSigBuilder::calcs(const char *data, int length) {
	BigInteger HXRm(0L);
	{
		MuSigHashBuilder hashBuilder;
		hashBuilder.add(&this->X);
		hashBuilder.add(&this->R);
		hashBuilder.addArray(data, length);
		hashBuilder.buildHash();

		HXRm = hashBuilder.getResultAsBigInteger(); //.mod(Secp256k1Point::n);
	}

	int maxLoop = this->signers->size();
	for(int i = 0; i != maxLoop; ++i){
		IMuSigSigner* signer = this->signers->get(i);

		BigInteger si = signer->gets(&HXRm, &this->L);
		s.addSelf(si);
	}

	s.modSelf(Secp256k1Point::n);
}

} /* namespace codablecash */
