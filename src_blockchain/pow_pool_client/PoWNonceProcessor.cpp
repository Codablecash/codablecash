/*
 * PoWNonceProcessor.cpp
 *
 *  Created on: Apr 16, 2026
 *      Author: iizuka
 */

#include "pow_pool_client/PoWNonceProcessor.h"
#include "pow_pool_client/PoWWorkerClient.h"
#include "pow_pool_client/PoWNonceCalcCommandMessage.h"
#include "pow_pool_client/PoWNonceProcessorParam.h"

#include "pow_pool/PoWRequestData.h"

#include "pow/PoWNonceResult.h"

#include "bc/ISystemLogger.h"

#include "procesor_multi/MultipleCommandProcessor.h"

#include "base/UnicodeString.h"

#include "base_thread/SysMutex.h"

#include "numeric/BigInteger.h"

#include "base_thread/StackUnlocker.h"

namespace codablecash {

PoWNonceProcessor::PoWNonceProcessor(PoWWorkerClient* client,  int size, ISystemLogger* logger) {
	this->logger = logger;

	this->param = new PoWNonceProcessorParam(client, this);
	UnicodeString prefix(PREFIX);
	this->cmdProcessor = new MultipleCommandProcessor(this->param, size, logger, &prefix);

	this->data = nullptr;
	this->solt = nullptr;
	this->nonce = nullptr;
	this->difficulty = nullptr;

	this->done = false;
	this->interrapted = false;
	this->mutex = new SysMutex();
}

PoWNonceProcessor::~PoWNonceProcessor() {
	shutdown();
	delete this->cmdProcessor;
	delete this->param;
	delete this->data;
	delete this->solt;

	delete this->nonce;
	delete this->difficulty;
	delete this->mutex;
}

void PoWNonceProcessor::start(const PoWRequestData *data, const BigInteger* solt, int numHash) {
	this->done = false;
	this->interrapted = false;

	delete this->data;
	delete this->solt;
	this->data = new PoWRequestData(*data);
	this->solt = new BigInteger(*solt);

	int maxLoop = this->cmdProcessor->size();
	int perUnits = numHash / maxLoop;
	int mod = numHash % maxLoop;

	for(int i = 0; i != maxLoop; ++i){
		int loops = perUnits;
		if(mod != 0){
			loops++;
			mod--;
		}

		PoWNonceCalcCommandMessage* cmd = new PoWNonceCalcCommandMessage(this->data, this->solt, loops, i);
		this->cmdProcessor->addCommandMessage(cmd, i);
	}

	this->cmdProcessor->start();
}

void PoWNonceProcessor::shutdown() {
	if(this->cmdProcessor != nullptr){
		this->cmdProcessor->shutdown();
		delete this->cmdProcessor;
		this->cmdProcessor = nullptr;
	}
}

bool PoWNonceProcessor::onNonceCalculated(const PoWNonce *nonce, const BigInteger* diff) {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	if(!this->done && !this->interrapted && (this->nonce == nullptr || diff->compareTo(*this->difficulty) > 0)){
		delete this->nonce;
		this->nonce = new PoWNonce(*nonce);
		delete this->difficulty;
		this->difficulty = new BigInteger(*diff);

		BigInteger* targetDiff = this->data->getTargetDifficulty();
		if(diff->compareTo(*targetDiff) >= 0){
			this->done = true;
		}
	}

	return !this->done && !this->interrapted;
}

void PoWNonceProcessor::stop(uint64_t height) {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	if(this->data != nullptr){
		uint64_t lasteight = this->data->getLastBlockHeight();
		if(lasteight >= height){
			this->interrapted = true;
		}
	}
}

} /* namespace codablecash */
