/*
 * PoWClientMainThread.cpp
 *
 *  Created on: Apr 15, 2026
 *      Author: iizuka
 */

#include "pow_pool_client/PoWClientMainThread.h"
#include "pow_pool_client/PoWWorkerClient.h"

#include "pow_pool_client_cmd/PoWPoolStatusCommand.h"
#include "pow_pool_client_cmd/PoWPoolStatusCommandResponse.h"

#include "pow_pool/PoWRequestStatusData.h"
#include "pow_pool/PoWRequest2Client.h"

#include "base_thread/SynchronizedLock.h"
#include "base_thread/StackUnlocker.h"

#include "osenv/funcs.h"

#include "pubsub/P2pHandshake.h"

#include "bc/ExceptionThrower.h"

#include "ipconnect/UnexpectedProtocolException.h"

#include "pow_pool_client_cmd/PoWPoolCheckDataCommand.h"

#include "pow_pool_client_cmd/PoWPoolCheckDataCommandResponse.h"

#include "base/StackRelease.h"

#include "bc/ISystemLogger.h"

#include "pow_pool_client/PoWNonceProcessor.h"

#include "pow_pool/PoWRequestData.h"

#include "pow_pool_client_cmd/PoWPoolNonceCalculatedCommand.h"

#include "pubsub_cmd/OkPubsubResponse.h"

#include "pow/PoWNonce.h"
namespace codablecash {

PoWClientMainThread::PoWClientMainThread(PoWWorkerClient* powClient, ISystemLogger* logger, const UnicodeString* name)
		: AbstractThreadRunner(name) {
	this->powClient = powClient;
	this->logger = logger;
	this->running = true;
	this->lock = new SynchronizedLock();

	this->loopCount = 100;
}

PoWClientMainThread::~PoWClientMainThread() {
	delete this->lock;
}

void PoWClientMainThread::process() noexcept {
	while(isRunning()){
		mainProcess();

		Os::usleep(100 * 1000);
	}
}

void PoWClientMainThread::mainProcess() {
	// get status(suspend or not)
	bool ruuning = getMainStatus();

	if(ruuning){
		// check hash data
		PoWRequest2Client* request = getCalculationRequest(); __STP(request);

		// calculate use PoWWorkerClient#start
		if(request != nullptr && !request->isWaiting()){
			calculate(request);
		}
	}
}

void PoWClientMainThread::calculate(PoWRequest2Client *request) {
	int numThread = this->powClient->getNumThread();
	int numHash = this->powClient->getNumHashOnce();

	PoWRequestData* data = request->getRequestData();
	BigInteger* solt = request->getSolt();

	this->powClient->start(data, solt, numHash);

	// report
	const PoWNonce* nonce = this->powClient->getNonce();

	{
		P2pHandshake* handshake = this->powClient->getHandshake();

		PoWPoolNonceCalculatedCommand cmd;
		PoWWorkerId* clientId = this->powClient->getClientId();
		cmd.setClientId(clientId);
		cmd.setPoWNonce(nonce);

		AbstractCommandResponse* res = handshake->publishCommand(&cmd); __STP(res);
		{
			UnicodeString* resstr = res->toString(); __STP(resstr);
			this->logger->debugLog(ISystemLogger::DEBUG_POOL_TRANSFER_RESPONSE, resstr, __FILE__, __LINE__);
		}

		OkPubsubResponse* okResponse = dynamic_cast<OkPubsubResponse*>(res);
		ExceptionThrower<UnexpectedProtocolException>::throwExceptionIfCondition(okResponse == nullptr, L"Unexpected response comes from server.", __FILE__, __LINE__);
	}
}

bool PoWClientMainThread::getMainStatus() {
	P2pHandshake* handshake = this->powClient->getHandshake();

	PoWPoolStatusCommand cmd;
	PoWWorkerId* clientId = this->powClient->getClientId();
	cmd.setClientId(clientId);

	AbstractCommandResponse* res = handshake->publishCommand(&cmd); __STP(res);
	{
		UnicodeString* resstr = res->toString(); __STP(resstr);
		this->logger->debugLog(ISystemLogger::DEBUG_POOL_TRANSFER_RESPONSE, resstr, __FILE__, __LINE__);
	}

	PoWPoolStatusCommandResponse* poolStatusRes = dynamic_cast<PoWPoolStatusCommandResponse*>(res);

	ExceptionThrower<UnexpectedProtocolException>::throwExceptionIfCondition(poolStatusRes == nullptr, L"Unexpected response comes from server.", __FILE__, __LINE__);

	PoWRequestStatusData* data = poolStatusRes->getPoWRequestStatusData();
	bool waiting = data->isWaiting();

	return !waiting;
}

PoWRequest2Client* PoWClientMainThread::getCalculationRequest() {
	P2pHandshake* handshake = this->powClient->getHandshake();

	PoWPoolCheckDataCommand cmd;
	PoWWorkerId* clientId = this->powClient->getClientId();
	cmd.setClientId(clientId);

	AbstractCommandResponse* res = handshake->publishCommand(&cmd); __STP(res);
	{
		UnicodeString* resstr = res->toString(); __STP(resstr);
		this->logger->debugLog(ISystemLogger::DEBUG_POOL_TRANSFER_RESPONSE, resstr, __FILE__, __LINE__);
	}

	PoWPoolCheckDataCommandResponse* hashRequestRes = dynamic_cast<PoWPoolCheckDataCommandResponse*>(res);

	ExceptionThrower<UnexpectedProtocolException>::throwExceptionIfCondition(hashRequestRes == nullptr, L"Unexpected response comes from server.", __FILE__, __LINE__);

	PoWRequest2Client* ret = hashRequestRes->getRequest();
	return new PoWRequest2Client(*ret);
}


void PoWClientMainThread::setRunning(bool bl) noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);
	this->running = bl;
	this->lock->notifyAll();
}

bool PoWClientMainThread::isRunning() const noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);
	return this->running;
}
} /* namespace codablecash */
