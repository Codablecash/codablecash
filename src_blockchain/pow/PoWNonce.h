/*
 * PoWNonce.h
 *
 *  Created on: 2023/03/21
 *      Author: iizuka
 */

#ifndef POW_POWNONCE_H_
#define POW_POWNONCE_H_

#include <cstdint>

#include "filestore_block/IBlockObject.h"

namespace alinous {
class BigInteger;
class ByteBuffer;
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class BlockHeaderId;
class PoWNonceResult;
class BlockMerkleRoot;

class PoWNonce : public alinous::IBlockObject {
public:
	static const BigInteger* getMaxBigInt() noexcept;
	static const PoWNonce MAX_NONCE;

	PoWNonce(const PoWNonce& inst);
	explicit PoWNonce(const BigInteger* nonce);
	virtual ~PoWNonce();

	static PoWNonce* createRandomNonce() noexcept;
	static PoWNonce* createRandomNonce(const BigInteger* solt) noexcept;

	PoWNonceResult* calcResult(const BlockHeaderId* lastHeaderId, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm) const;

	ByteBuffer* tobyte32Buffer() const;

	BigInteger getDifficulty() const noexcept;
	static BigInteger calcNecessaryDifficulty(BigInteger& hashRate, uint64_t milliseconds);
	static BigInteger calcDifficulty2Ntimes(BigInteger& diff);

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static PoWNonce* createFromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	int compareTo(const PoWNonce* other) const noexcept;

	const BigInteger* getValue() const noexcept {
		return this->value;
	}

protected:
	BigInteger* value;
};

} /* namespace codablecash */

#endif /* POW_POWNONCE_H_ */
