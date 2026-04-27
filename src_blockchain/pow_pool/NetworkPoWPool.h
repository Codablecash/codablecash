/*
 * NetworkPoWPool.h
 *
 *  Created on: Apr 13, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_NETWORKPOWPOOL_H_
#define POW_POOL_NETWORKPOWPOOL_H_

#include "pow/AbstractPoWCalculator.h"

#include "pubsub/IPubsubCommandExecutor.h"

#include "procesor/ICommandParameter.h"

namespace alinous {
class BigInteger;
class SystemTimestamp;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class BlockHeaderId;
class BlockMerkleRoot;
class PoWRequestStatus;
class P2pServer;
class PoWNonce;
class PoWNonceResult;
class PoWManager;
class PoWRequestStatusData;
class PoWRequest2Client;
class MultipleCommandProcessor;

class NetworkPoWPool : public AbstractPoWCalculator, public IPubsubCommandExecutor, public ICommandParameter {
public:
	explicit NetworkPoWPool(PoWManager* powManager);
	virtual ~NetworkPoWPool();

	virtual void calculate(uint64_t lastBlockHeight, const BlockHeaderId* lastBlockId
			, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm, const BigInteger* targetDifficulty);
	virtual void init(ISystemLogger* logger);
	virtual void shutdown();

	virtual void suspend();
	virtual void resume();
	virtual bool isSuspendStatus() const noexcept;

	virtual void startNetwork();

	void setProtocol(int type);
	void setServerHost(const UnicodeString* host);
	void setPort(int port);

	PoWRequestStatus* getRequestStatus() const noexcept {
		return this->status;
	}

	P2pServer* getP2pServer() const noexcept {
		return this->p2pServer;
	}
	ISystemLogger* getlogger() const noexcept {
		return this->logger;
	}

	void onClientCalculatedNonce(const PoWRequest2Client* request, const PoWNonce *nonce);

private:
	void notifyClientsMined(uint64_t height, const BlockHeaderId *bid, const PoWNonce *nonce);

private:
	PoWRequestStatus* status;
	ISystemLogger *logger;

	int protocol;
	UnicodeString* srvhost;
	int port;

	P2pServer* p2pServer;

	PoWManager* powManager;

	bool suspended;

	// network transfer
	MultipleCommandProcessor* executor;
};

} /* namespace codablecash */

#endif /* POW_POOL_NETWORKPOWPOOL_H_ */
