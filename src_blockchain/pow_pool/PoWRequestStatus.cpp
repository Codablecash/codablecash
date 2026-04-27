/*
 * PoWRequestStatus.cpp
 *
 *  Created on: Apr 14, 2026
 *      Author: iizuka
 */

#include "pow_pool/PoWRequestStatus.h"
#include "pow_pool/PoWRequestData.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"

#include "pow_pool/PoWRequestStatusData.h"

#include "pow_pool/PoWRequest2Client.h"

#include "numeric/BigInteger.h"

#include "pow/PoWNonce.h"

namespace codablecash {

PoWRequestStatus::PoWRequestStatus() {
	this->data = nullptr;
	this->mutex = new SysMutex();
	this->waiting = false;
	this->solt = 0;
}

PoWRequestStatus::~PoWRequestStatus() {
	delete this->data;
	delete this->mutex;
}

bool PoWRequestStatus::isWaiting() const noexcept {
	StackUnlocker __unlock(this->mutex, __FILE__, __LINE__);

	return this->waiting;
}

void PoWRequestStatus::setWaiting(bool bl) {
	StackUnlocker __unlock(this->mutex, __FILE__, __LINE__);

	this->waiting = bl;
}

void PoWRequestStatus::request(uint64_t lastBlockHeight, const BlockHeaderId *lastBlockId, const BlockMerkleRoot *merkleRoot,
		const SystemTimestamp *tm, const BigInteger *targetDifficulty) {
	StackUnlocker __unlock(this->mutex, __FILE__, __LINE__);

	delete this->data, this->data = nullptr;
	this->data = new PoWRequestData();
	this->data->init(lastBlockHeight, lastBlockId, merkleRoot, tm , targetDifficulty);

	this->waiting = false;
}

PoWRequestStatusData* PoWRequestStatus::getStatusData() const noexcept {
	StackUnlocker __unlock(this->mutex, __FILE__, __LINE__);

	PoWRequestStatusData* ret = new PoWRequestStatusData();
	ret->setWaiting(this->waiting);

	return ret;
}

PoWRequest2Client* PoWRequestStatus::getClientPoWRequest() {
	StackUnlocker __unlock(this->mutex, __FILE__, __LINE__);

	PoWRequest2Client* req = new PoWRequest2Client();
	req->setRequestData(this->data);

	const BigInteger* max = PoWNonce::getMaxBigInt();
	BigInteger nonceSolt = BigInteger::ramdom(BigInteger(0L), BigInteger(*max));

	BigInteger s(this->solt++);
	nonceSolt.addSelf(s);

	req->setSolt(&nonceSolt);

	return req;
}

const BigInteger* PoWRequestStatus::getTargetDifficulty() const noexcept {
	StackUnlocker __unlock(this->mutex, __FILE__, __LINE__);
	return this->data->getTargetDifficulty();
}

} /* namespace codablecash */
