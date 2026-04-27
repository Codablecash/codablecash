/*
 * InstanceSessionContext.cpp
 *
 *  Created on: Feb 18, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_index/InstanceSessionContext.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "bc_base/BinaryUtils.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"

namespace codablecash {

InstanceSessionContext::InstanceSessionContext(const InstanceSessionContext &inst) {
	this->callSerial = inst.callSerial;
	this->trxId = inst.trxId != nullptr ? dynamic_cast<CdbDatabaseSessionId*>(inst.trxId->copyData()) : nullptr;
}

InstanceSessionContext::InstanceSessionContext() {
	this->callSerial = 0;
	this->trxId = nullptr;
}

InstanceSessionContext::~InstanceSessionContext() {
	delete this->trxId;
}

void InstanceSessionContext::setCallSerial(uint64_t callSerial) {
	this->callSerial = callSerial;
}

void InstanceSessionContext::setTrxId(const CdbDatabaseSessionId *trxId) {
	delete this->trxId;
	this->trxId = dynamic_cast<CdbDatabaseSessionId*>(trxId->copyData());
}

int InstanceSessionContext::binarySize() const {
	BinaryUtils::checkNotNull(this->trxId);

	int total = sizeof(this->callSerial);
	total += this->trxId->binarySize();

	return total;
}

void InstanceSessionContext::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->trxId);

	out->putLong(this->callSerial);
	this->trxId->toBinary(out);
}

InstanceSessionContext* InstanceSessionContext::createFromBinary(ByteBuffer *in) {
	InstanceSessionContext* inst = new InstanceSessionContext(); __STP(inst);

	inst->callSerial = in->getLong();
	inst->trxId = CdbDatabaseSessionId::createFromBinary(in);

	return __STP_MV(inst);
}

IBlockObject* InstanceSessionContext::copyData() const noexcept {
	return new InstanceSessionContext(*this);
}

int SessionContextReverseCompare::operator ()(const InstanceSessionContext *const a,
		const InstanceSessionContext *const b) const noexcept {
	uint64_t aserial = a->getCallSerial();
	uint64_t bserial = b->getCallSerial();

	return bserial == aserial ? 0 : (aserial > bserial ? -1 : 1);
}

} /* namespace codablecash */
