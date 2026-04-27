/*
 * PoWRequestStatus.h
 *
 *  Created on: Apr 14, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_POWREQUESTSTATUS_H_
#define POW_POOL_POWREQUESTSTATUS_H_
#include <cstdint>

namespace alinous {
class SysMutex;
class SystemTimestamp;
class BigInteger;
}
using namespace alinous;

namespace codablecash {

class BlockHeaderId;
class BlockMerkleRoot;
class PoWRequestData;
class PoWRequestStatusData;
class PoWRequest2Client;

class PoWRequestStatus {
public:
	PoWRequestStatus();
	virtual ~PoWRequestStatus();

	bool isWaiting() const noexcept;
	void setWaiting(bool bl);

	void request(uint64_t lastBlockHeight, const BlockHeaderId* lastBlockId
			, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm, const BigInteger* targetDifficulty);

	PoWRequestStatusData* getStatusData() const noexcept;
	PoWRequest2Client* getClientPoWRequest();

	const BigInteger* getTargetDifficulty() const noexcept;


private:
	bool waiting;
	PoWRequestData* data;
	SysMutex* mutex;

	uint64_t solt;
};

} /* namespace codablecash */

#endif /* POW_POOL_POWREQUESTSTATUS_H_ */
