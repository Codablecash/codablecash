/*
 * MuSig.cpp
 *
 *  Created on: 2023/02/02
 *      Author: iizuka
 */

#include "musig/MuSig.h"
#include "musig/MuSigHashBuilder.h"

#include "numeric/BigInteger.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"


namespace codablecash {

MuSig& MuSig::operator =(const MuSig &inst) {
	if(this != &inst){
		this->R = inst.R;
		this->s = inst.s;

		int maxLoop = inst.XiList->size();
		for(int i = 0; i != maxLoop; ++i){
			const Secp256k1Point* pt = inst.XiList->get(i);
			this->XiList->addElement(new Secp256k1Point(*pt));
		}
	}
	return(*this);
}

MuSig::MuSig(const MuSig &inst)
		: R(inst.R), s(inst.s) {
	this->XiList = new ArrayList<Secp256k1Point>();
	int maxLoop = inst.XiList->size();
	for(int i = 0; i != maxLoop; ++i){
		const Secp256k1Point* pt = inst.XiList->get(i);
		this->XiList->addElement(new Secp256k1Point(*pt));
	}
}

MuSig::MuSig(const Secp256k1Point R, const BigInteger s)
		: R(R), s(s) {
	this->XiList = new ArrayList<Secp256k1Point>();
}

MuSig::~MuSig() {
	this->XiList->deleteElements();
	delete this->XiList;
}

void MuSig::addXi(const Secp256k1Point Xi) noexcept {
	this->XiList->addElement(new Secp256k1Point(Xi));
}

/**
 *  * Call L = H(X1,X2,…)
 * Call X the sum of all H(L,Xi)Xi
 *
 * sG = R + H(X,R,m)X
 */
bool MuSig::verify(const char *data, int length) const noexcept {
	Secp256k1Point X = calcX();

	Secp256k1Point G;
	Secp256k1Point sG = G.multiple(this->s);

	BigInteger HXRm(0L);
	{
		MuSigHashBuilder hashBuilder;
		hashBuilder.add(&X);
		hashBuilder.add(&this->R);
		hashBuilder.addArray(data, length);
		hashBuilder.buildHash();
		HXRm = hashBuilder.getResultAsBigInteger();
	}

	Secp256k1Point result = X.multiple(HXRm);
	result = result.add(this->R);

	return result.equals(sG);
}

/**
 * Call X the sum of all H(L,Xi)Xi
 */
Secp256k1Point MuSig::calcX() const noexcept {
	BigInteger L = calcL();

	Secp256k1Point result(BigInteger(L"0", 10), BigInteger(L"0", 10)); // O point

	int maxLoop = this->XiList->size();
	for(int i = 0; i != maxLoop; ++i){
		Secp256k1Point* Xi = this->XiList->get(i);

		MuSigHashBuilder hashBuilder;
		hashBuilder.addBigInteger(&L);
		hashBuilder.add(Xi);
		hashBuilder.buildHash();

		BigInteger hash = hashBuilder.getResultAsBigInteger().mod(Secp256k1Point::n);
		Secp256k1Point value = Xi->multiple(hash);

		result = result.add(value);
	}

	return result;
}

/**
 * 	Call L = H(X1,X2,…)
 */
BigInteger MuSig::calcL() const noexcept {
	MuSigHashBuilder hashBuilder;

	int maxLoop = this->XiList->size();
	for(int i = 0; i != maxLoop; ++i){
		Secp256k1Point* Xi = this->XiList->get(i);
		hashBuilder.add(Xi);
	}
	hashBuilder.buildHash();

	BigInteger L = hashBuilder.getResultAsBigInteger();
	//L.modSelf(Secp256k1Point::n);

	return L;
}

UnicodeString* MuSig::toString() const {
	UnicodeString* ret = new UnicodeString(L"");

	{
		UnicodeString* rstr = this->R.toString(16); __STP(rstr);
		ret->append(L"R : ").append(rstr);
	}

	return ret;
}

} /* namespace codablecash */
