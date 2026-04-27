/*
 * PoWWorkerClient.h
 *
 *  Created on: Apr 13, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_POWWORKERCLIENT_H_
#define POW_POOL_CLIENT_POWWORKERCLIENT_H_

#include "pubsub/IPubsubCommandExecutor.h"
#include <cstdint>

namespace alinous {
class UnicodeString;
class SysMutex;
class BigInteger;
}
using namespace alinous;

namespace codablecash {

class PoWWorkerId;
class PoWClientMainThread;
class ISystemLogger;
class P2pHandshake;
class PoWPubsubCommandExecutor;
class PoWRequestData;
class PoWNonceProcessor;
class PoWNonce;

class PoWWorkerClient : public IPubsubCommandExecutor {
public:
	PoWWorkerClient(ISystemLogger* logger, int numThreads, int hashOnce);
	virtual ~PoWWorkerClient();

	void start(int protocol, const UnicodeString *host, int port);
	void shutdown();
	void shutdownMainThread();
	void shutdownProcessor();
	void start(const PoWRequestData* data, const BigInteger* solt, int numHash);

	P2pHandshake* getHandshake() const noexcept {
		return this->handshake;
	}

	PoWWorkerId* getClientId() const noexcept {
		return this->clientId;
	}

	int getNumThread() const noexcept {
		return this->numThreads;
	}
	int getNumHashOnce() const noexcept {
		return this->hashOnce;
	}

	const PoWNonce* getNonce() const noexcept;
	void stop(uint64_t height) const noexcept;

private:
	void __shutdownProcessor();

private:
	PoWWorkerId* clientId;
	int numThreads;
	int hashOnce;

	PoWClientMainThread* mainThread;

	ISystemLogger* logger;

	P2pHandshake *handshake;
	PoWPubsubCommandExecutor* exec;

	SysMutex* mutex;
	PoWNonceProcessor* currentProcessor;

};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_POWWORKERCLIENT_H_ */
