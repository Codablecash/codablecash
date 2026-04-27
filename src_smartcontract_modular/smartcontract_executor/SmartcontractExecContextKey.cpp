/*
 * SmartcontractExecContextKey.cpp
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor/SmartcontractExecContextKey.h"
#include "smartcontract_executor/SmartcontractExecContextKeyFactory.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"

namespace codablecash {

SmartcontractExecContextKey::SmartcontractExecContextKey(const SmartcontractExecContextKey &inst) {
	this->trxId = inst.trxId != nullptr ? dynamic_cast<CdbDatabaseSessionId*>(inst.trxId->copyData()) : nullptr;
}

SmartcontractExecContextKey::SmartcontractExecContextKey() {
	this->trxId = nullptr;
}

SmartcontractExecContextKey::~SmartcontractExecContextKey() {
	delete this->trxId;
}

int SmartcontractExecContextKey::binarySize() const {
	BinaryUtils::checkNotNull(this->trxId);

	int size = sizeof(uint32_t);
	size += this->trxId->binarySize();

	return size;
}

void SmartcontractExecContextKey::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->trxId);

	out->putInt(SmartcontractExecContextKeyFactory::SMARTCONTRACT_EXEC_ID_KEY);
	this->trxId->toBinary(out);
}

SmartcontractExecContextKey* SmartcontractExecContextKey::fromBinary(ByteBuffer *in) {
	CdbDatabaseSessionId* trxId = CdbDatabaseSessionId::createFromBinary(in); __STP(trxId);

	SmartcontractExecContextKey* key = new SmartcontractExecContextKey();
	key->setTransactionId(trxId);

	return key;
}

int SmartcontractExecContextKey::compareTo(const AbstractBtreeKey *key) const noexcept {
	if(key->isInfinity()){
		return -1;
	}
	else if(key->isNull()){
		return 1;
	}

	const SmartcontractExecContextKey* other = dynamic_cast<const SmartcontractExecContextKey*>(key);
	assert(other != nullptr);

	return this->trxId->compareTo(other->trxId);
}

AbstractBtreeKey* SmartcontractExecContextKey::clone() const noexcept {
	return new SmartcontractExecContextKey(*this);
}

void SmartcontractExecContextKey::setTransactionId(const CdbDatabaseSessionId *trxId) {
	delete this->trxId;
	this->trxId = dynamic_cast<CdbDatabaseSessionId*>(trxId->copyData());
}

} /* namespace codablecash */
