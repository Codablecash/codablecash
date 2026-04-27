/*
 * PoWManager.cpp
 *
 *  Created on: 2023/03/22
 *      Author: iizuka
 */

#include "pow/PoWManager.h"
#include "pow/PoWCalculator.h"
#include "pow/PoWStartCalculateCommandMessage.h"
#include "pow/PoWGenerateBlockRequestCommandMessage.h"

#include "base/UnicodeString.h"

#include "procesor/MessageProcessor.h"

#include "bc_block/BlockHeaderId.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"

#include "bc/DebugDefaultLogger.h"

#include "bc_block_generator/MiningConfig.h"


namespace codablecash {

const UnicodeString PoWManager::THREAD_NAME(L"PoW_Main");

PoWManager::PoWManager(ISystemLogger* logger, const MiningConfig *config) {
	this->miningConfig = new MiningConfig(*config);

	this->processor = nullptr;
	this->calculator = nullptr;
	this->lastBlockId = nullptr;
	this->mutexBlockId = new SysMutex();
	this->generator = nullptr;
	this->logger = logger;
}

PoWManager::~PoWManager() {
	shutdown();

	delete this->lastBlockId;
	delete this->mutexBlockId;

	this->generator = nullptr;
	delete this->miningConfig;
}

void PoWManager::start() {
	this->calculator = this->miningConfig->getPoWCalculator(this);

	this->calculator->setPowManager(this);
	this->calculator->init(this->logger);

	this->processor = new MessageProcessor(&THREAD_NAME, this->logger);
	this->processor->setParam(this);
	this->processor->start();

	this->calculator->startNetwork();
}

void PoWManager::shutdown() {
	if(this->calculator != nullptr){
		this->calculator->shutdown();
	}
	if(this->processor != nullptr){
		this->processor->shutdown();
	}

	delete this->processor;
	this->processor = nullptr;
	delete this->calculator;
	this->calculator = nullptr;
}

void PoWManager::requestNewHeaderId() {
	PoWStartCalculateCommandMessage* cmd = new PoWStartCalculateCommandMessage();
	this->processor->addCommandMessage(cmd);
}

void PoWManager::resendMinerRequest() {
	PoWStartCalculateCommandMessage* cmd = new PoWStartCalculateCommandMessage();
	this->processor->addCommandMessage(cmd);
}

void PoWManager::onNonceCalculated(uint64_t height, const BlockHeaderId *bid, const PoWNonce *nonce) {
	bool isNonceCalculated = false;
	{
		StackUnlocker unlocker(this->mutexBlockId, __FILE__, __LINE__);
		isNonceCalculated = (this->lastBlockId == nullptr || this->lastBlockId->compareTo(bid) != 0);
	}

	UnicodeString dmsg(L"processing onNonceCalculated.");
	dmsg.append(L"At height ");
	dmsg.append((int64_t)height);

	dmsg.append(L" isNonceCalculated ");
	isNonceCalculated ? dmsg.append(L"true") : dmsg.append(L"false");

	this->logger->debugLog(ISystemLogger::DEBUG_POW_CALC_THREAD,&dmsg, __FILE__, __LINE__);

	if(isNonceCalculated){
		fireNonceCalculated(height, bid, nonce);

		{
			StackUnlocker unlocker(this->mutexBlockId, __FILE__, __LINE__);
			delete this->lastBlockId;
			this->lastBlockId = dynamic_cast<BlockHeaderId*>(bid->copyData());
		}
	}
}

void PoWManager::fireNonceCalculated(uint64_t height, const BlockHeaderId *bid,	const PoWNonce *nonce) {
	PoWGenerateBlockRequestCommandMessage* cmd = new PoWGenerateBlockRequestCommandMessage(height, bid, nonce);
	this->processor->addCommandMessage(cmd);
}


void PoWManager::suspend() {
	this->calculator->suspend();
}

void PoWManager::resume() {
	this->calculator->resume();
}

bool PoWManager::isSuspendStatus() const noexcept {
	return this->calculator->isSuspendStatus();
}

} /* namespace codablecash */
