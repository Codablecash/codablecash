/*
 * PoWWorkerClient.cpp
 *
 *  Created on: Apr 13, 2026
 *      Author: iizuka
 */

#include "pow_pool_client/PoWWorkerClient.h"
#include "pow_pool_client/PoWWorkerId.h"
#include "pow_pool_client/PoWClientMainThread.h"
#include "pow_pool_client/PoWPubsubCommandExecutor.h"

#include "pubsub/PubSubId.h"
#include "pubsub/P2pHandshake.h"

#include "base/StackRelease.h"

#include "pow_pool_client/PoWNonceProcessor.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"

using alinous::StackUnlocker;
namespace codablecash {

PoWWorkerClient::PoWWorkerClient(ISystemLogger* logger, int numThreads, int hashOnce) {
	this->numThreads = numThreads;
	this->hashOnce = hashOnce;
	this->clientId = PoWWorkerId::createRandomId();
	this->mainThread = nullptr;
	this->logger = logger;
	this->handshake = nullptr;
	this->exec = new PoWPubsubCommandExecutor(this);

	this->mutex = new SysMutex();
	this->currentProcessor = nullptr;
}

PoWWorkerClient::~PoWWorkerClient() {
	shutdownMainThread();
	shutdownProcessor();
	shutdown();
	delete this->clientId;
	delete this->mainThread;
	delete this->exec;
	delete this->mutex;
}

void PoWWorkerClient::start(int protocol, const UnicodeString *host, int port) {
	PubSubId* psId = PubSubId::createNewId(); __STP(psId);

	// establish handshake
	this->handshake = new P2pHandshake(psId, logger);
	this->handshake->connect(protocol, host, port, this->exec, true);

	// start thread
	UnicodeString str(L"PoWClientMain");
	this->mainThread = new PoWClientMainThread(this, this->logger, &str);

	this->mainThread->start();
}

void PoWWorkerClient::shutdownMainThread() {
	if(this->mainThread != nullptr){
		this->mainThread->setRunning(false);
		this->mainThread->join();
		delete this->mainThread;
		this->mainThread = nullptr;
	}
}

void PoWWorkerClient::shutdown() {
	if(this->handshake != nullptr){
		this->handshake->dispose();
		delete this->handshake;
		this->handshake = nullptr;
	}
}

void PoWWorkerClient::shutdownProcessor() {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);
	__shutdownProcessor();
}

void PoWWorkerClient::__shutdownProcessor() {
	if(this->currentProcessor != nullptr){
		this->currentProcessor->stop(0);
		this->currentProcessor->shutdown();
		delete this->currentProcessor;
		this->currentProcessor = nullptr;
	}
}

void PoWWorkerClient::start(const PoWRequestData *data, const BigInteger *solt, int numHash) {
	shutdownProcessor(); // reset

	{
		StackUnlocker __lock(this->mutex, __FILE__, __LINE__);
		this->currentProcessor = new PoWNonceProcessor(this, this->numThreads, this->logger);

		this->currentProcessor->start(data, solt, numHash);
	}
}

const PoWNonce* PoWWorkerClient::getNonce() const noexcept {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);
	return this->currentProcessor != nullptr ? this->currentProcessor->getNonce() : nullptr;
}

void PoWWorkerClient::stop(uint64_t height) const noexcept {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);
	if(this->currentProcessor != nullptr){
		this->currentProcessor->stop(height);
	}
}

} /* namespace codablecash */
