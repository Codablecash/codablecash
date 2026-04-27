/*
 * SmartcontractAddress.cpp
 *
 *  Created on: Oct 2, 2025
 *      Author: iizuka
 */

#include "transaction/AbstractSmartcontractAddress.h"

#include "base/StackRelease.h"

#include "crypto/Sha256.h"
#include "crypto/RipeMd160.h"

#include "ecda/Secp256k1CompressedPoint.h"

namespace codablecash {

AbstractSmartcontractAddress::AbstractSmartcontractAddress() : AbstractAddress(0) {
	this->pubkey = nullptr;
}

AbstractSmartcontractAddress::AbstractSmartcontractAddress(uint16_t zone, Secp256k1CompressedPoint *pubkey)  : AbstractAddress(zone) {
	this->pubkey = pubkey;
}

AbstractSmartcontractAddress::~AbstractSmartcontractAddress() {
	delete this->pubkey;
}

ByteBuffer* AbstractSmartcontractAddress::getBodyPart() const noexcept {
	int size = this->pubkey->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(size, true); __STP(buff);
	this->pubkey->toBinary(buff);

	// sha256
	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);

	// RIPEMD-160
	ByteBuffer* ripe = RipeMd160::encode(sha);

	return ripe;
}

int AbstractSmartcontractAddress::binarySize() const {
	int total = sizeof(uint8_t);
	total += sizeof(this->zone);
	total += this->pubkey->binarySize();

	return total;
}

void AbstractSmartcontractAddress::toBinary(ByteBuffer *out) const {
	out->put(getType());

	out->putShort(this->zone);

	this->pubkey->toBinary(out);
}

void AbstractSmartcontractAddress::fromBinary(ByteBuffer *in) {
	this->zone = in->getShort();

	this->pubkey = Secp256k1CompressedPoint::fromBinary(in);
}

int AbstractSmartcontractAddress::compareTo(const AbstractSmartcontractAddress *other) const noexcept {
	return this->pubkey->compareTo(other->pubkey);
}

} /* namespace codablecash */
