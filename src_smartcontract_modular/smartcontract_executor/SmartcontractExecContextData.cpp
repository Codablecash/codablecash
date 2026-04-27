/*
 * SmartcontractExecContextData.cpp
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor/SmartcontractExecContextData.h"

#include "smartcontract_executor_operations_data/AbstractSmartcontractOperationData.h"

#include "bc_trx/TransactionId.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "bc_block/BlockHeaderId.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"


namespace codablecash {

SmartcontractExecContextData::SmartcontractExecContextData(const SmartcontractExecContextData &inst) {
	this->callSerial = inst.callSerial;
	this->height = inst.height;
	this->blockHeaderId = inst.blockHeaderId != nullptr ? dynamic_cast<BlockHeaderId*>(inst.blockHeaderId->copyData()) : nullptr;
	this->opData = inst.opData != nullptr ? dynamic_cast<AbstractSmartcontractOperationData*>(inst.opData->copyData()) : nullptr;

	this->lastTrxId = inst.lastTrxId != nullptr ? dynamic_cast<CdbDatabaseSessionId*>(inst.lastTrxId->copyData()) : nullptr;
	this->finalized = inst.finalized;
}

SmartcontractExecContextData::SmartcontractExecContextData() {
	this->callSerial = 0;
	this->height = 0;
	this->blockHeaderId = nullptr;
	this->opData = nullptr;


	this->lastTrxId = nullptr;
	this->finalized = false;
}

SmartcontractExecContextData::~SmartcontractExecContextData() {
	delete this->blockHeaderId;
	delete this->opData;
	delete this->lastTrxId;
}

int SmartcontractExecContextData::binarySize() const {
	BinaryUtils::checkNotNull(this->blockHeaderId);
	BinaryUtils::checkNotNull(this->opData);


	int total = sizeof(this->callSerial);
	total += sizeof(this->height);
	total += this->blockHeaderId->binarySize();
	total += this->opData->binarySize();

	if(this->callSerial > 1){
		total += this->lastTrxId->binarySize();
	}

	total += sizeof(uint8_t);

	return total;
}

void SmartcontractExecContextData::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->blockHeaderId);
	BinaryUtils::checkNotNull(this->opData);

	out->putLong(this->callSerial);
	out->putLong(this->height);
	this->blockHeaderId->toBinary(out);
	this->opData->toBinary(out);

	if(this->callSerial > 1){
		this->lastTrxId->toBinary(out);
	}

	out->put(this->finalized ? 1 : 0);
}

SmartcontractExecContextData* SmartcontractExecContextData::createFromBinary(ByteBuffer *in) {
	SmartcontractExecContextData* inst = new SmartcontractExecContextData(); __STP(inst);

	inst->callSerial = in->getLong();
	inst->height = in->getLong();
	inst->blockHeaderId = BlockHeaderId::fromBinary(in);

	inst->opData = AbstractSmartcontractOperationData::createFromBinary(in);

	if(inst->callSerial > 1){
		inst->lastTrxId = CdbDatabaseSessionId::createFromBinary(in);
	}

	bool bl = in->get();
	inst->finalized = (bl > 0);

	return __STP_MV(inst);
}

IBlockObject* SmartcontractExecContextData::copyData() const noexcept {
	return new SmartcontractExecContextData(*this);
}

void SmartcontractExecContextData::setOperationData(const AbstractSmartcontractOperationData *opData) {
	delete this->opData;
	this->opData = dynamic_cast<AbstractSmartcontractOperationData*>(opData->copyData());
}

void SmartcontractExecContextData::setCallSerial(uint64_t callSerial) {
	this->callSerial = callSerial;
}

void SmartcontractExecContextData::setHeight(uint64_t height) {
	this->height = height;
}

void SmartcontractExecContextData::setBlockHead(const BlockHeaderId *blockHeaderId) {
	delete this->blockHeaderId, this->blockHeaderId = nullptr;
	this->blockHeaderId = dynamic_cast<BlockHeaderId*>(blockHeaderId->copyData());
}

void SmartcontractExecContextData::setLastSessionId(const CdbDatabaseSessionId *lastTrxId) {
	delete this->lastTrxId, this->lastTrxId = nullptr;
	this->lastTrxId = lastTrxId != nullptr ? dynamic_cast<CdbDatabaseSessionId*>(lastTrxId->copyData()) : nullptr;
}

void SmartcontractExecContextData::setFinalized(bool finalized) {
	this->finalized = finalized;
}

} /* namespace codablecash */
