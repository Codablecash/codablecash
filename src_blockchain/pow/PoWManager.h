/*
 * PoWManager.h
 *
 *  Created on: 2023/03/22
 *      Author: iizuka
 */

#ifndef POW_POWMANAGER_H_
#define POW_POWMANAGER_H_

#include "procesor/ICommandParameter.h"

#include "base/ArrayList.h"

#include <cstdint>

namespace alinous {
class UnicodeString;
class BigInteger;
class SysMutex;
}
using namespace alinous;

namespace codablecash {

class MessageProcessor;
class AbstractPoWCalculator;
class PoWNonce;
class BlockHeaderId;
class BlockGenerator;
class ISystemLogger;
class MiningConfig;


class PoWManager : public ICommandParameter {
public:
	static const UnicodeString THREAD_NAME;

	explicit PoWManager(ISystemLogger* logger, const MiningConfig *config);
	virtual ~PoWManager();

	void setBlockGenerator(BlockGenerator* generator) noexcept {
		this->generator = generator;
	}
	BlockGenerator* getBlockGenerator() const noexcept {
		return this->generator;
	}

	void start();
	void shutdown();

	void suspend();
	void resume();
	bool isSuspendStatus() const noexcept;

	void requestNewHeaderId();
	void resendMinerRequest();

	void onNonceCalculated(uint64_t height, const BlockHeaderId *bid, const PoWNonce *nonce);

	AbstractPoWCalculator* getCalculator() const noexcept {
		return this->calculator;
	}

private:
	void fireNonceCalculated(uint64_t height, const BlockHeaderId *bid, const PoWNonce *nonce);

private:
	MiningConfig* miningConfig;

	ISystemLogger* logger;
	MessageProcessor* processor;
	AbstractPoWCalculator* calculator;

	BlockGenerator* generator;

	SysMutex* mutexBlockId;
	BlockHeaderId *lastBlockId;
};

} /* namespace codablecash */

#endif /* POW_POWMANAGER_H_ */
