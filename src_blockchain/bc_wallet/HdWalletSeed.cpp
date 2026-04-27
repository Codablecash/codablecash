/*
 * HdWalletSeed.cpp
 *
 *  Created on: 2023/01/23
 *      Author: iizuka
 */

#include "bc_wallet/HdWalletSeed.h"
#include "bc_wallet_encoder/IWalletDataEncoder.h"

#include "numeric/BigInteger.h"

#include "base/StackRelease.h"

#include "crypto/Sha256.h"

#include <cstdint>


namespace codablecash {




HdWalletSeed::HdWalletSeed() : Abstract32BytesId() {
}

HdWalletSeed::HdWalletSeed(const char *binary, int length) : Abstract32BytesId(binary, length) {
}

HdWalletSeed::~HdWalletSeed() {
}

HdWalletSeed* HdWalletSeed::newSeed() noexcept {
	ByteBuffer* buff = makeRandom16Bytes(); __STP(buff);
	buff->position(0);

	HdWalletSeed* walletSeed = new HdWalletSeed();
	walletSeed->id = ByteBuffer::wrapWithEndian(buff->array(), buff->capacity(), true);

	return walletSeed;

}

IBlockObject* HdWalletSeed::copyData() const noexcept {
	return new HdWalletSeed(*this);
}

HdWalletSeed* HdWalletSeed::encodedSeed(const IWalletDataEncoder *encoder) const noexcept {
	return encoder->encode(this);
}

HdWalletSeed* HdWalletSeed::indexedSeed(int accountIndex) const noexcept {
	int cap = this->id->limit() + sizeof(uint64_t);
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

	buff->putLong(accountIndex);

	this->id->position(0);
	buff->put(this->id);

	ByteBuffer* id = Sha256::sha256(buff, true);

	HdWalletSeed* walletSeed = new HdWalletSeed();
	walletSeed->id = id;

	return walletSeed;
}

} /* namespace codablecash */
