/*
 * PoWNonceProcessor.h
 *
 *  Created on: Apr 16, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_POWNONCEPROCESSOR_H_
#define POW_POOL_CLIENT_POWNONCEPROCESSOR_H_
#include <cstdint>

namespace alinous {
class BigInteger;
class SysMutex;
}
using namespace alinous;

namespace codablecash {

class MultipleCommandProcessor;
class PoWWorkerClient;
class ISystemLogger;
class PoWRequestData;
class PoWNonceProcessorParam;
class PoWNonce;

class PoWNonceProcessor {
public:
	static const constexpr wchar_t* PREFIX = L"PoWProc";

	PoWNonceProcessor(PoWWorkerClient* client,  int size, ISystemLogger* logger);
	virtual ~PoWNonceProcessor();

	void start(const PoWRequestData* data, const BigInteger* solt, int numHash);
	void shutdown();
	void stop(uint64_t height);

	bool onNonceCalculated(const PoWNonce* nonce, const BigInteger* diff);

	const PoWNonce* getNonce() const noexcept {
		return this->nonce;
	}

private:
	MultipleCommandProcessor* cmdProcessor;
	ISystemLogger* logger;

	PoWNonceProcessorParam* param;

	// current/last data
	PoWRequestData* data;
	BigInteger* solt;

	// result
	PoWNonce *nonce; // base nonce used in calc
	BigInteger* difficulty; // difficulty of nonce's result

	// flags
	bool done;
	bool interrapted;
	SysMutex* mutex;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_POWNONCEPROCESSOR_H_ */
