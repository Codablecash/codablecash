/*
 * PoWNonceProcessorParam.h
 *
 *  Created on: Apr 17, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_POWNONCEPROCESSORPARAM_H_
#define POW_POOL_CLIENT_POWNONCEPROCESSORPARAM_H_

#include "procesor/ICommandParameter.h"

namespace codablecash {

class PoWWorkerClient;
class PoWNonceProcessor;

class PoWNonceProcessorParam : public ICommandParameter {
public:
	PoWNonceProcessorParam(PoWWorkerClient* client, PoWNonceProcessor* processor);
	virtual ~PoWNonceProcessorParam();

	PoWWorkerClient* getClient() const noexcept {
		return this->client;
	}
	PoWNonceProcessor* getProcessor() const noexcept {
		return this->processor;
	}

private:
	PoWWorkerClient* client;
	PoWNonceProcessor* processor;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_POWNONCEPROCESSORPARAM_H_ */
