/*
 * NetworkClientCommandProcessorThread.cpp
 *
 *  Created on: Mar 26, 2025
 *      Author: iizuka
 */

#include "bc_wallet_net_processor/NetworkClientCommandProcessorThread.h"
#include "bc_wallet_net_processor/ClientCommandsQueueProcessor.h"

#include "base_thread/SynchronizedLock.h"
#include "base_thread/StackUnlocker.h"

#include "base/StackRelease.h"
#include "bc_wallet_net_cmd/AbstractClientQueueCommand.h"

#include "base/Exception.h"

#include "bc/ISystemLogger.h"
namespace codablecash {

NetworkClientCommandProcessorThread::NetworkClientCommandProcessorThread(NetworkWallet* wallet, ClientCommandsQueueProcessor* queue, bool suspend, ISystemLogger* logger, const UnicodeString* name)
		: AbstractThreadRunner(name) {
	this->logger = logger;
	this->queue = queue;
	this->running = false;
	this->suspend = suspend;
	this->status = STATUS_STOPPED;

	this->lock = new SynchronizedLock();
	this->wallet = wallet;
}

NetworkClientCommandProcessorThread::~NetworkClientCommandProcessorThread() {
	delete this->lock;

	this->wallet = nullptr;
}

void NetworkClientCommandProcessorThread::process() noexcept {
	while(isRunning() || !this->queue->isEmpty()){
		AbstractClientQueueCommand* cmd = nullptr;

		{
			StackUnlocker unlocker(lock, __FILE__, __LINE__);

			this->status = STATUS_RUNNING;

			if(!this->suspend && !this->queue->isEmpty()){
				cmd = this->queue->fetchFirst();
			}
			else{ // empty
				this->status = STATUS_WAITING;
				if(this->running){
					lock->wait();
				}
				this->status = STATUS_RUNNING;
			}

			lock->notifyAll();
		}

		// process
		if(cmd != nullptr){
			__STP(cmd);
			processCommand(cmd);
		}
	}
}

void NetworkClientCommandProcessorThread::processCommand(const AbstractClientQueueCommand *cmd) const {
	try{
		cmd->process(this->wallet);
	}
	catch(Exception* e){
		this->logger->logException(e);
		delete e;
	}

}

void NetworkClientCommandProcessorThread::setRunning(bool bl) noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	this->running = bl;
	this->lock->notifyAll();
}

bool NetworkClientCommandProcessorThread::isRunning() const noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	return this->running;
}

int NetworkClientCommandProcessorThread::getStatus() const noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	return this->status;
}

void NetworkClientCommandProcessorThread::__setSuspend(bool suspend) {
	this->suspend = suspend;
	this->lock->notifyAll();
}
/*
bool NetworkClientCommandProcessorThread::isSuspend() const {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	return __isSuspended();
}*/

void NetworkClientCommandProcessorThread::putCommnad(const AbstractClientQueueCommand *cmd) {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	this->queue->addCommand(cmd);
}

} /* namespace codablecash */
