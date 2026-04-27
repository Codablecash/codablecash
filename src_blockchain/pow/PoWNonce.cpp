/*
 * PoWNonce.cpp
 *
 *  Created on: 2023/03/21
 *      Author: iizuka
 */

#include "pow/PoWNonce.h"
#include "pow/PoWNonceResult.h"

#include "numeric/BigInteger.h"

#include "ecda/Secp256k1Point.h"

#include "pow_hash/PowRandomHashManager.h"

#include "bc_block/BlockHeaderId.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "bc_block/BlockMerkleRoot.h"

#include "base_timestamp/SystemTimestamp.h"

#include "crypto/Sha256.h"
namespace codablecash {

const BigInteger* PoWNonce::getMaxBigInt() noexcept {
	/**
	 * 32 bytes value
	 * FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
	 */
	static const BigInteger MAX_NONCE_BIG_INT(L"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16);

	return &MAX_NONCE_BIG_INT;
}
const PoWNonce PoWNonce::MAX_NONCE(getMaxBigInt());

PoWNonce::PoWNonce(const PoWNonce &inst) {
	this->value = new BigInteger(*inst.value);
}

PoWNonce::PoWNonce(const BigInteger* nonce) {
	this->value = new BigInteger(*nonce);
}

PoWNonce::~PoWNonce() {
	delete this->value;
}
PoWNonce* PoWNonce::createRandomNonce() noexcept {
	const BigInteger* max = getMaxBigInt();
	BigInteger nonce = BigInteger::ramdom(BigInteger(0L), BigInteger(*max));

	return new PoWNonce(&nonce);
}

PoWNonce* PoWNonce::createRandomNonce(const BigInteger *solt) noexcept {
	const BigInteger* max = getMaxBigInt();
	BigInteger nonce = BigInteger::ramdom(BigInteger(0L), BigInteger(*max));

	nonce = nonce.multiply(*solt).mod(*max);

	return new PoWNonce(&nonce);
}

PoWNonceResult* PoWNonce::calcResult(const BlockHeaderId *lastHeaderId, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm) const {
	PowRandomHashManager manager;

	ByteBuffer* out = nullptr;
	{
		// add timestamp
		int cap = lastHeaderId->binarySize();
		cap += merkleRoot->binarySize();
		cap += tm->binarySize();

		ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
		lastHeaderId->toBinary(buff);
		merkleRoot->toBinary(buff);
		tm->toBinary(buff);

		buff->position(0);

		out = Sha256::sha256(buff, true);
	}
	 __STP(out);


	const char* bin = (const char*)out->array();
	int length = out->limit();

	// Blockhash must be 32 bytes length
	BigInteger* blockHash = BigInteger::fromBinary(bin, length); __STP(blockHash);

	ByteBuffer* buff = manager.calculate(blockHash, this->value); __STP(buff);

	BigInteger* res = BigInteger::fromBinary((const char*)buff->array(), buff->limit()); __STP(res);

	return new PoWNonceResult(res);
}

ByteBuffer* PoWNonce::tobyte32Buffer() const {
	ByteBuffer* buff = this->value->toBinary(); __STP(buff);

	return BigInteger::padBuffer(buff, 32);
}

BigInteger PoWNonce::getDifficulty() const noexcept {
	BigInteger diff = MAX_NONCE.getMaxBigInt()->subtract(*this->value);

	return diff;
}

BigInteger PoWNonce::calcNecessaryDifficulty(BigInteger& hashRate,	uint64_t milliseconds) {
	BigInteger intervalMillSec = BigInteger(milliseconds);
	BigInteger N = hashRate.multiply(intervalMillSec).divide(BigInteger(1000));
	if(N.equals(&BigInteger::ZERO)){
		N = BigInteger(1L);
	}

	BigInteger max = *getMaxBigInt();
	BigInteger Nmax = max.multiply(N);

	BigInteger diff = Nmax.subtract(max).divide(N);

	return diff;
}

BigInteger PoWNonce::calcDifficulty2Ntimes(BigInteger &diff) {
	BigInteger max = *getMaxBigInt();
	BigInteger max_diff = max.subtract(diff);

	BigInteger N = max.divide(max_diff);
	return N;
}

int PoWNonce::binarySize() const {
	return sizeof(uint8_t) * 32;
}

void PoWNonce::toBinary(ByteBuffer *out) const {
	ByteBuffer* buff = tobyte32Buffer(); __STP(buff);
	buff->position(0);

	out->put(buff->array(), buff->limit());
}

IBlockObject* PoWNonce::copyData() const noexcept {
	return new PoWNonce(this->value);
}

PoWNonce* PoWNonce::createFromBinary(ByteBuffer *in) {
	uint8_t tmp[32];
	in->get(tmp, 32);

	BigInteger* bint =  BigInteger::fromBinary((const char*)tmp, 32); __STP(bint);

	return new PoWNonce(bint);
}

int PoWNonce::compareTo(const PoWNonce *other) const noexcept {
	return this->value->compareTo(*other->value);
}

} /* namespace codablecash */
