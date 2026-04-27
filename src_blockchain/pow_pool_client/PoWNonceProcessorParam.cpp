/*
 * PoWNonceProcessorParam.cpp
 *
 *  Created on: Apr 17, 2026
 *      Author: iizuka
 */

#include "pow_pool_client/PoWNonceProcessorParam.h"

namespace codablecash {

PoWNonceProcessorParam::PoWNonceProcessorParam(PoWWorkerClient* client, PoWNonceProcessor* processor) {
	this->client = client;
	this->processor = processor;
}

PoWNonceProcessorParam::~PoWNonceProcessorParam() {

}

} /* namespace codablecash */
