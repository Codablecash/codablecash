/*
 * P2pRequestProcessor.cpp
 *
 *  Created on: 2023/09/02
 *      Author: iizuka
 */

#include "bc_p2p_processor/P2pRequestProcessor.h"
#include "bc_p2p_processor/NetworkInfoProcessor.h"
#include "bc_p2p_processor/NetworkTransferProcessor.h"
#include "bc_p2p_processor/P2pRequestQueueProcessor.h"

#include "bc_p2p_processor_pending/PendingProcessorCommandsQueue.h"
#include "bc_p2p_processor_pending/PendingCommandData.h"

#include "bc_network/NodeIdentifierSource.h"

#include "data_history/DataHistoryStore.h"
#include "data_history/AbstractTransferedData.h"
#include "data_history/TransferedDataId.h"
#include "data_history/DataHistory.h"

#include "base/StackRelease.h"
#include "base/Exception.h"

#include "base_io/File.h"

#include "bc/ISystemLogger.h"
#include "bc/ExceptionThrower.h"

#include "command_queue/CommandQueueData.h"

#include "base_thread/StackUnlocker.h"
#include "base_thread/SynchronizedLock.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_p2p_cmd_node/AbstractNodeCommand.h"

#include "bc_p2p/BlochchainP2pManager.h"
#include "bc_p2p/StackHandshakeReleaser.h"


#include "bc_p2p_cmd/SignatureVerifivcationErrorException.h"

#include "pubsub_cmd/AbstractCommandResponse.h"

namespace codablecash {

P2pRequestProcessor::P2pRequestProcessor(const File* baseDir, BlochchainP2pManager* p2pManager, P2pDnsManager* p2pDnsManager, CodablecashSystemParam* config, ISystemLogger* logger) {
	this->config = config;
	this->logger = logger;

	this->baseDir = baseDir->get(BASE_DIR);
	this->p2pManager = p2pManager;
	this->p2pDnsManager = p2pDnsManager;

	this->networkKey = nullptr;

	this->infoProcessor = nullptr;
	this->transferProcessor = nullptr;

	this->queueProcessor = nullptr;
	this->historyStore = nullptr;
	this->pendingQueue = nullptr;
	this->nodeName = nullptr;
}

P2pRequestProcessor::~P2pRequestProcessor() {
	close();

	delete this->baseDir;
	delete this->networkKey;
	delete this->nodeName;
}

void P2pRequestProcessor::createBlank(CodablecashNodeInstance *inst) {
	{
		P2pRequestQueueProcessor processor(this->baseDir, this->p2pManager, this->transferProcessor, inst, this->logger);
		processor.createBlankDatabase();
	}
	{
		DataHistory store(this->baseDir);
		store.createBlankDatabase();
	}
	{
		PendingProcessorCommandsQueue queue(this->baseDir);
		queue.createBlankDatabase();
		queue.close();
	}

}

void P2pRequestProcessor::open(const NodeIdentifierSource *networkKey, CodablecashNodeInstance *inst, bool suspend) {
	this->networkKey = dynamic_cast<NodeIdentifierSource*>(networkKey->copyData());

	int numThreads = this->config->getThreadsNetworkInfoTransferProcessor();
	this->infoProcessor = new NetworkInfoProcessor(this, numThreads, this->logger);
	this->infoProcessor->start();

	numThreads = this->config->getThreadsNodeTransferProcessor();
	this->transferProcessor = new NetworkTransferProcessor(this, numThreads, this->logger);
	this->transferProcessor->start();

	this->queueProcessor = new P2pRequestQueueProcessor(this->baseDir, this->p2pManager, this->transferProcessor, inst, this->logger);
	this->queueProcessor->open(suspend);

	this->historyStore = new DataHistory(this->baseDir);
	this->historyStore->open();

	this->pendingQueue = new PendingProcessorCommandsQueue(this->baseDir);
	this->pendingQueue->open();
}

void P2pRequestProcessor::close() {
	if(this->queueProcessor != nullptr){
		this->queueProcessor->close();
		delete this->queueProcessor;
		this->queueProcessor = nullptr;
	}

	if(this->infoProcessor != nullptr){
		this->infoProcessor->shutdown();
		delete this->infoProcessor;
		this->infoProcessor = nullptr;
	}

	if(this->transferProcessor != nullptr){
		this->transferProcessor->shutdown();
		delete this->transferProcessor;
		this->transferProcessor = nullptr;
	}

	if(this->historyStore != nullptr){
		this->historyStore->close();
		delete this->historyStore;
		this->historyStore = nullptr;
	}

	if(this->pendingQueue != nullptr){
		this->pendingQueue->close();
		delete this->pendingQueue;
		this->pendingQueue = nullptr;
	}
}

bool P2pRequestProcessor::hasHistory(const AbstractTransferedData *data) {
	TransferedDataId* dataId = data->getTransferedDataId(); __STP(dataId);

	return this->historyStore->hasId(dataId);
}

void P2pRequestProcessor::addHistory(const AbstractTransferedData *data) {
	TransferedDataId* dataId = data->getTransferedDataId(); __STP(dataId);

	this->historyStore->add(dataId, data);
}

void P2pRequestProcessor::putQueue(const PubSubId *pubsubId, const AbstructNodeQueueCommand *command) {
	CommandQueueData data;
	data.setPublishId(pubsubId);
	data.setCommand(command);

	this->queueProcessor->put(&data);
}

/*void P2pRequestProcessor::setSuspend(bool suspend) {
	this->queueProcessor->setSuspend(suspend);
}
*/


bool P2pRequestProcessor::isSuspended() const noexcept {
	SynchronizedLock* lock = getSynchrinizedLock();
	StackUnlocker unlocker(lock, __FILE__, __LINE__);

	return __isSuspended();
}

bool P2pRequestProcessor::__isSuspended() const noexcept {
	return this->queueProcessor->__isSuspended();
}

SynchronizedLock* P2pRequestProcessor::getSynchrinizedLock() const noexcept {
	return this->queueProcessor->getSynchronizedLock();
}

void P2pRequestProcessor::__putPendingCommand(const AbstractNodeCommand *commnad, const PubSubId *pubsubId) {
	PendingCommandData data;
	data.setPubsubId(pubsubId);
	data.setNodeCommand(commnad);

	this->pendingQueue->put(&data);
}

void P2pRequestProcessor::resume(CodablecashNodeInstance* inst) {
	SynchronizedLock* lock = getSynchrinizedLock();

	while(!isPendingQueueEmptyWithResume(lock)){
		PendingCommandData* data = fetchFirstPendingData(lock); __STP(data);

		try{
			processData(inst, data);
		}
		catch(Exception* e){
			__STP(e);
			this->logger->logException(e);
		}
	}
}

void P2pRequestProcessor::processData(CodablecashNodeInstance *inst, PendingCommandData *data) {
	const PubSubId* pubsubId = data->getPubSubId();
	const AbstractNodeCommand* command = data->getNodeCommand();

	BlockchainNodeHandshake* nodeHandShake = this->p2pManager->getNodeHandshake(pubsubId);
	StackHandshakeReleaser __releaser(nodeHandShake);

	ExceptionThrower<SignatureVerifivcationErrorException>::throwExceptionIfCondition(nodeHandShake == nullptr, L"Node is not connected.", __FILE__, __LINE__);

	AbstractCommandResponse* response = command->executeAsNode(nodeHandShake, inst, true); __STP(response);

	UnicodeString* str = response->toString(); __STP(str);
	logger->debugLog(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE, str, __FILE__, __LINE__);

}

bool P2pRequestProcessor::isPendingQueueEmptyWithResume(SynchronizedLock *lock) const {
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

PendingCommandData* P2pRequestProcessor::fetchFirstPendingData(SynchronizedLock *lock) const {
	StackUnlocker unlocker(lock, __FILE__, __LINE__);

	return this->pendingQueue->fetchFirst();
}

void P2pRequestProcessor::setNodeName(const UnicodeString *name) noexcept {
	delete this->nodeName;
	this->nodeName = new UnicodeString(name);
}

} /* namespace codablecash */
