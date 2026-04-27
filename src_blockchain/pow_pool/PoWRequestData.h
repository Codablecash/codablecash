/*
 * PoWRequestData.h
 *
 *  Created on: Apr 14, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_POWREQUESTDATA_H_
#define POW_POOL_POWREQUESTDATA_H_

#include <cstdint>

namespace alinous {
class ByteBuffer;
class BigInteger;
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class BlockHeaderId;
class BlockMerkleRoot;


class PoWRequestData {
public:
	PoWRequestData(const PoWRequestData& inst);
	PoWRequestData();
	virtual ~PoWRequestData();

	void init(uint64_t lastBlockHeight, const BlockHeaderId *lastBlockId, const BlockMerkleRoot *merkleRoot,
			const SystemTimestamp *tm, const BigInteger *targetDifficulty);

	uint64_t getLastBlockHeight() const noexcept {
		return this->lastBlockHeight;
	}
	BlockHeaderId* getLastBlockId() const noexcept {
		return this->lastBlockId;
	}
	BlockMerkleRoot* getMerkleRoot() const noexcept {
		return this->merkleRoot;
	}
	SystemTimestamp* getTimestamp() const noexcept {
		return this->tm;
	}
	BigInteger* getTargetDifficulty() const noexcept {
		return this->targetDifficulty;
	}

	int binarySize() const;
	void toBinary(ByteBuffer* out) const;
	static PoWRequestData* fromBinary(ByteBuffer* in);

private:
	uint64_t lastBlockHeight;
	BlockHeaderId *lastBlockId;
	BlockMerkleRoot *merkleRoot;
	SystemTimestamp *tm;
	BigInteger *targetDifficulty;
};

} /* namespace codablecash */

#endif /* POW_POOL_POWREQUESTDATA_H_ */
