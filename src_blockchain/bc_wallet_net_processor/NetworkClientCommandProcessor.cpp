/*
 * NetworkClientCommandProcessor.cpp
 *
 *  Created on: Mar 21, 2025
 *      Author: iizuka
 */

#include "bc_wallet_net_processor/NetworkClientCommandProcessor.h"
#include "bc_wallet_net_processor/NetworkClientCommandListner.h"
#include "bc_wallet_net_processor/ClientCommandsQueueProcessor.h"

#include "bc_wallet_net_cmd_queue/PendingClientCommandsQueue.h"
#include "bc_wallet_net_cmd_queue/ClientCommandsQueueData.h"

#include "bc_wallet_net/NetworkWallet.h"

#include "bc_p2p_cmd_client_notify/ClientExecutor.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"
#include "base_thread/SynchronizedLock.h"

#include "base_io/File.h"

#include "base/StackRelease.h"
#include "base/Exception.h"

#include "bc/ISystemLogger.h"
#include "bc_wallet_net_cmd/AbstractClientQueueCommand.h"


namespace codablecash {

NetworkClientCommandProcessor::NetworkClientCommandProcessor(NetworkWallet* networkWallet, ISystemLogger* logger) {
	this->logger = logger;
	this->networkWallet = networkWallet;
	this->clientExec = new ClientExecutor();

	this->listner = new NetworkClientCommandListner(this);
	this->clientExec->addListner(this->listner);

	this->queueProcessor = nullptr;
	this->pendingQueue = nullptr;
}

NetworkClientCommandProcessor::~NetworkClientCommandProcessor() {
	if(this->queueProcessor != nullptr){
		this->queueProcessor->close();
		delete this->queueProcessor;
		this->queueProcessor = nullptr;
	}
	if(this->pendingQueue != nullptr){
		this->pendingQueue->close();
		delete this->pendingQueue;
		this->pendingQueue = nullptr;
	}

	this->networkWallet = nullptr;
	delete this->clientExec;

	delete this->listner;
}

void NetworkClientCommandProcessor::init() {
	if(this->queueProcessor == nullptr && this->pendingQueue == nullptr){
		const File* baseDir = networkWallet->getBaseDir();

		File* dir = baseDir->get(PROCESSER_DIR); __STP(dir);
		dir->deleteDir();
		dir->mkdirs();

		this->queueProcessor = new ClientCommandsQueueProcessor(this->networkWallet, dir, this->logger);
		this->pendingQueue = new PendingClientCommandsQueue(dir);
	}
}


void NetworkClientCommandProcessor::fireExecuteCommand(const PubSubId* pubsubId, const AbstractPubSubCommand *cmd) {
}

void NetworkClientCommandProcessor::fireOnExeptionOnSubscriber(const PubSubId* pubsubId, const Exception *e) {
}

void NetworkClientCommandProcessor::fireOnSubscribeEnds(const PubSubId* pubsubId) {
}

IPubsubCommandExecutor* NetworkClientCommandProcessor::getExecutor() const noexcept {
	return this->clientExec;
}

void NetworkClientCommandProcessor::createBlank() {
	this->queueProcessor->createBlankDatabase();
	this->pendingQueue->createBlankDatabase();
}

void NetworkClientCommandProcessor::startProcessors(bool suspend) {
	this->queueProcessor->open(suspend);
	this->pendingQueue->open();
}

void NetworkClientCommandProcessor::resumeRequestProcessor() {
	SynchronizedLock* lock = this->queueProcessor->getSynchrinizedLock();

	while(!isPendingQueueEmptyWithResume(lock)){
		ClientCommandsQueueData* data = fetchFirstPendingData(lock); __STP(data);

		try{
			processPendingData(data);
		}
		catch(Exception* e){
			__STP(e);
			this->logger->logException(e);
		}
	}
}

ClientCommandsQueueData* NetworkClientCommandProcessor::fetchFirstPendingData(SynchronizedLock *lock) const {
	ClientCommandsQueueData* data = nullptr;
	{
		StackUnlocker unlocker(lock, __FILE__, __LINE__);
		data = this->pendingQueue->fetchFirst();
	}


	return data;
}

bool NetworkClientCommandProcessor::isPendingQueueEmptyWithResume(SynchronizedLock *lock) const {
	bool result = false;
	{
		StackUnlocker unlocker(lock, __FILE__, __LINE__);

		result = this->pendingQueue->isEmpty();
		if(result){
			this->queueProcessor->__setSuspend(false);
		}
	}

	return result;
}

void NetworkClientCommandProcessor::processPendingData(ClientCommandsQueueData *data) {
	AbstractClientQueueCommand* cmd = data->toClientCommand(); __STP(cmd);

	cmd->process(this->networkWallet);
}

bool NetworkClientCommandProcessor::__isSuspended() const {
	return this->queueProcessor->__isSuspended();
}

bool NetworkClientCommandProcessor::isSuspended() const {
	SynchronizedLock* lock = getQueueSynchrinizedLock();

	StackUnlocker unlocker(lock, __FILE__, __LINE__);
	return this->queueProcessor->__isSuspended();
}

SynchronizedLock* NetworkClientCommandProcessor::getQueueSynchrinizedLock() const noexcept {
	return this->queueProcessor->getSynchrinizedLock();
}

void NetworkClientCommandProcessor::addPendingQueue(const ClientCommandsQueueData *queueData) {
	this->pendingQueue->addCommnadData(queueData);
}

void NetworkClientCommandProcessor::addClientCommand(const AbstractClientQueueCommand *clientCommnad) {
	SynchronizedLock* lock = getQueueSynchrinizedLock();

	{
		StackUnlocker unlocker(lock, __FILE__, __LINE__);
		this->queueProcessor->addCommand(clientCommnad);

		// notify all
		lock->notifyAll();
	}
}

void NetworkClientCommandProcessor::shurdownProcessors() noexcept {
	// remove all execute listners
	this->clientExec->clearAllListners();

	// wait for queue commands to be empty
	this->queueProcessor->close();

}

WalletNetworkManager* NetworkClientCommandProcessor::getWalletNetworkManager() const noexcept {
	return this->networkWallet->getWalletNetworkManager();
}

} /* namespace codablecash */
