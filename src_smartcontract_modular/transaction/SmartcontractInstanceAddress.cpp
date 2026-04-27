/*
 * SmartcontractInstanceAddress.cpp
 *
 *  Created on: Oct 2, 2025
 *      Author: iizuka
 */

#include "transaction/SmartcontractInstanceAddress.h"

#include "ecda/Secp256k1CompressedPoint.h"
#include "ecda/ScPublicKey.h"
#include "ecda/ScPrivateKey.h"

#include "bc_wallet/HdWalletSeed.h"

#include "base/StackRelease.h"


namespace codablecash {

const char* SmartcontractInstanceAddress::PREFIX = "ci";

SmartcontractInstanceAddress::SmartcontractInstanceAddress() : AbstractSmartcontractAddress() {

}

SmartcontractInstanceAddress::SmartcontractInstanceAddress(uint16_t zone, Secp256k1CompressedPoint *pubkey)
	: AbstractSmartcontractAddress(zone, pubkey) {

}

SmartcontractInstanceAddress::~SmartcontractInstanceAddress() {

}

const char* SmartcontractInstanceAddress::getPrefix() const noexcept {
	return SmartcontractInstanceAddress::PREFIX;
}

IBlockObject* SmartcontractInstanceAddress::copyData() const noexcept {
	return new SmartcontractInstanceAddress(this->zone, dynamic_cast<Secp256k1CompressedPoint*>(this->pubkey->copyData()));
}

SmartcontractInstanceAddress* SmartcontractInstanceAddress::createAddress(uint16_t zone, const ScPublicKey *publicKey) {
	Secp256k1CompressedPoint pubkey = Secp256k1CompressedPoint::compress(publicKey);
	SmartcontractInstanceAddress* address = new SmartcontractInstanceAddress(zone, dynamic_cast<Secp256k1CompressedPoint*>(pubkey.copyData()));

	return address;
}

SmartcontractInstanceAddress* SmartcontractInstanceAddress::createAddress(uint16_t zone) {
	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);

	ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);
	ScPublicKey publicKey = privateKey->generatePublicKey();

	return createAddress(zone, &publicKey);
}

} /* namespace codablecash */
