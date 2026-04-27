/*
 * AbstractPoWCalculator.h
 *
 *  Created on: 2023/03/22
 *      Author: iizuka
 */

#ifndef POW_ABSTRACTPOWCALCULATOR_H_
#define POW_ABSTRACTPOWCALCULATOR_H_

#include <cstdint>

namespace alinous {
class BigInteger;
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class BlockHeaderId;
class PoWManager;
class ISystemLogger;
class BlockMerkleRoot;

class AbstractPoWCalculator {
public:
	AbstractPoWCalculator();
	virtual ~AbstractPoWCalculator();

	virtual void calculate(uint64_t lastBlockHeight, const BlockHeaderId* lastBlockId
			, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm, const BigInteger* targetDifficulty) = 0;
	virtual void init(ISystemLogger* logger) = 0;
	virtual void shutdown() = 0;

	virtual void suspend() = 0;
	virtual void resume() = 0;
	virtual bool isSuspendStatus() const noexcept = 0;

	void setPowManager(PoWManager* powManager) noexcept;

	virtual void startNetwork(){};

protected:
	PoWManager* powManager;
};

} /* namespace codablecash */

#endif /* POW_ABSTRACTPOWCALCULATOR_H_ */
