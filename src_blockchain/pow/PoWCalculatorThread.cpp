/*
 * PoWCalculatorThread.cpp
 *
 *  Created on: 2023/03/22
 *      Author: iizuka
 */

#include "pow/PoWCalculatorThread.h"

#include "base/StackRelease.h"

#include "base_thread/StackUnlocker.h"
#include "base_thread/SynchronizedLock.h"

#include "base_timestamp/SystemTimestamp.h"

#include "bc_block/BlockHeaderId.h"

#include "numeric/BigInteger.h"

#include "pow/PoWNonce.h"
#include "pow/PoWNonceResult.h"
#include "pow/PoWManager.h"

#include "bc/ISystemLogger.h"

#include "bc_block/BlockMerkleRoot.h"

namespace codablecash {

PoWCalculatorThread::PoWCalculatorThread(PoWManager* powManager, const UnicodeString* name, ISystemLogger* logger)
			: AbstractThreadRunner(name) {
	this->running = false;
	this->suspended = false;
	this->status = STAT_STOPPED;
	this->lock = new SynchronizedLock();

	this->lastBlockId = nullptr;
	this->targetDifficulty = nullptr;
	this->lastBlockHeight = 0;

	this->merkleRoot = nullptr;
	this->timestamp = nullptr;

	this->powManager = powManager;

	this->logger = logger;
}

PoWCalculatorThread::~PoWCalculatorThread() {
	delete this->lock;
	delete this->lastBlockId;
	delete this->targetDifficulty;
	delete this->merkleRoot;
	delete this->timestamp;
}

void PoWCalculatorThread::process() noexcept {
	BlockHeaderId* bid = nullptr;
	BigInteger* tdiff = nullptr;
	uint64_t height = 0;
	BlockMerkleRoot* merkleRoot = nullptr;
	SystemTimestamp* tm = nullptr;

	while(isRunning()){
		{
			StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

			if(this->suspended || this->lastBlockId == nullptr){
				this->status = STAT_WAITING;
				if(this->running){
					lock->wait();
				}
				this->status = STAT_RUNNING;
			}
			else if(!this->suspended){
				bid = dynamic_cast<BlockHeaderId*>(this->lastBlockId->copyData());
				tdiff = new BigInteger(*this->targetDifficulty);
				height = this->lastBlockHeight;
				merkleRoot = dynamic_cast<BlockMerkleRoot*>(this->merkleRoot->copyData());
				tm = dynamic_cast<SystemTimestamp*>(this->timestamp->copyData());
			}
		}

		// calc one
		if(!isSuspend() && bid != nullptr){
			UnicodeString dmsg(L"Calclating....");
			dmsg.append(L"At height ");
			dmsg.append((int64_t)height);
			this->logger->debugLog(ISystemLogger::DEBUG_POW_CALC_THREAD, &dmsg, __FILE__, __LINE__);

			doCalc(height, 10, bid, tdiff, merkleRoot, tm);
		}

		delete bid; delete tdiff; delete merkleRoot; delete tm;
		bid = nullptr; tdiff = nullptr; merkleRoot = nullptr; tm = nullptr;
	}

	delete bid; delete tdiff; delete merkleRoot; delete tm;
}

void PoWCalculatorThread::doCalc(uint64_t height, int maxLoop, const BlockHeaderId* bid, const BigInteger* tdiff, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm) {
	for(int i = 0; !isSuspend() && i != maxLoop; ++i){
		PoWNonce* nonce = calcOne(bid, tdiff, merkleRoot, tm); __STP(nonce);
		if(nonce != nullptr){
			checkAndReportNonce(height, bid, tdiff, nonce);
			break;
		}
	}
}

void PoWCalculatorThread::checkAndReportNonce(uint64_t height, const BlockHeaderId *bid, const BigInteger *tdiff, const PoWNonce *nonce) {
	bool isCalculated = false;
	{
		StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

		isCalculated = (this->lastBlockId->compareTo(bid) == 0);
		if(isCalculated){
			delete this->lastBlockId;
			delete this->targetDifficulty;

			this->lastBlockId = nullptr;
			this->targetDifficulty = nullptr;

			UnicodeString dmsg(L"Reset calc lastBlockId.");
			dmsg.append(L"At height ");
			dmsg.append((int64_t)height);
			this->logger->debugLog(ISystemLogger::DEBUG_POW_CALC_THREAD,&dmsg, __FILE__, __LINE__);
		}
	}

	if(isCalculated){
		UnicodeString dmsg(L"onNonceCalculated.");
		dmsg.append(L"At height ");
		dmsg.append((int64_t)height);

		this->logger->debugLog(ISystemLogger::DEBUG_POW_CALC_THREAD,&dmsg, __FILE__, __LINE__);
		this->powManager->onNonceCalculated(height, bid, nonce);
	}
}

PoWNonce* PoWCalculatorThread::calcOne(const BlockHeaderId *blockHeaderId, const BigInteger *tdiff, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm) {
	PoWNonce* n = PoWNonce::createRandomNonce(); __STP(n);
	PoWNonceResult* result = n->calcResult(blockHeaderId, merkleRoot, tm); __STP(result);

	PoWNonce* ret = nullptr;

	BigInteger diff = result->getDifficulty();
	if(diff.compareTo(*tdiff) >= 0){
		ret = __STP_MV(n);
	}

	return ret;
}

void PoWCalculatorThread::request(uint64_t lastBlockHeight, const BlockHeaderId *lastBlockId, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm, const BigInteger *targetDifficulty) noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	UnicodeString message(L"  [PoW Request] Last Height: ");
	{
		UnicodeString* strId = lastBlockId->toString(); __STP(strId);

		message.append((int64_t)lastBlockHeight);
		message.append(L" Last Block Id: ").append(strId);
	}

	if(this->lastBlockId != nullptr && this->lastBlockId->equals(lastBlockId)){
		UnicodeString message2(L"   ***** ----> Skkipped");

		message2.append(&message);
		this->logger->log(&message2);
		return;
	}
	this->logger->log(&message);

	delete this->lastBlockId;
	delete this->targetDifficulty;
	delete this->merkleRoot;
	delete this->timestamp;

	this->lastBlockId = lastBlockId != nullptr ? dynamic_cast<BlockHeaderId*>(lastBlockId->copyData()) : nullptr;
	this->targetDifficulty = targetDifficulty != nullptr ? new BigInteger(*targetDifficulty) : nullptr;
	this->lastBlockHeight = lastBlockHeight;
	this->merkleRoot = merkleRoot != nullptr ? dynamic_cast<BlockMerkleRoot*>(merkleRoot->copyData()) : nullptr;
	this->timestamp = tm != nullptr ? dynamic_cast<SystemTimestamp*>(tm->copyData()) : nullptr;

	this->lock->notifyAll();
}

void PoWCalculatorThread::setRunning(bool bl) noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	this->running = bl;
	this->lock->notifyAll();
}

void PoWCalculatorThread::setSuspended(bool bl) noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	this->suspended = bl;
	this->lock->notifyAll();
}

bool PoWCalculatorThread::isSuspend() const noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	return this->suspended;
}

bool PoWCalculatorThread::isRunning() const noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	return this->running;
}

bool PoWCalculatorThread::isSuspendStatus() const noexcept {
	StackUnlocker unlocker(this->lock, __FILE__, __LINE__);

	return this->suspended && this->status == STAT_WAITING;
}



} /* namespace codablecash */
