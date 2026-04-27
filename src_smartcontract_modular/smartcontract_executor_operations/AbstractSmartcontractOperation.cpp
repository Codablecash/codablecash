/*
 * AbstractSmartcontractOperation.cpp
 *
 *  Created on: Jan 21, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_operations/AbstractSmartcontractOperation.h"

#include "bc_block/BlockHeaderId.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

namespace codablecash {

AbstractSmartcontractOperation::AbstractSmartcontractOperation() {
	this->trxId = nullptr;
	this->height = 0;
	this->blockHeaderId = nullptr;
	this->lastTrxId = nullptr;
}

AbstractSmartcontractOperation::~AbstractSmartcontractOperation() {
	delete this->trxId;
	delete this->blockHeaderId;
	delete this->lastTrxId;
}

void AbstractSmartcontractOperation::setSessionId(const CdbDatabaseSessionId *trxId) {
	this->trxId = dynamic_cast<CdbDatabaseSessionId*>(trxId->copyData());
}

void AbstractSmartcontractOperation::setHeaderId(uint64_t height, const BlockHeaderId *blockHeaderId) {
	this->height = height;

	delete this->blockHeaderId;
	this->blockHeaderId = dynamic_cast<BlockHeaderId*>(blockHeaderId->copyData());
}

void AbstractSmartcontractOperation::setLastTrxId(const CdbDatabaseSessionId *lastTrxId) {
	this->lastTrxId = dynamic_cast<CdbDatabaseSessionId*>(lastTrxId->copyData());
}

} /* namespace codablecash */
