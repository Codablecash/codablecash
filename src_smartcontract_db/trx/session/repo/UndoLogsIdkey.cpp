/*
 * UndoLogsIdkey.cpp
 *
 *  Created on: 2023/07/10
 *      Author: iizuka
 */

#include "trx/session/repo/UndoLogsIdkey.h"
#include "trx/session/repo/UndoLogsKeyFactory.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "base_io/ByteBuffer.h"

#include "engine/CdbOid.h"

#include "bc_base/BinaryUtils.h"

namespace codablecash {

UndoLogsIdkey::UndoLogsIdkey(const UndoLogsIdkey &inst) {
	this->sessionId = new CdbDatabaseSessionId(*inst.sessionId);
	this->undoLogSerial = new CdbOid(*inst.undoLogSerial);
}

UndoLogsIdkey::UndoLogsIdkey() {
	this->sessionId = nullptr;
	this->undoLogSerial = nullptr;
}

UndoLogsIdkey::~UndoLogsIdkey() {
	delete this->sessionId;
	delete this->undoLogSerial;
}

int UndoLogsIdkey::binarySize() const {
	BinaryUtils::checkNotNull(this->sessionId);
	BinaryUtils::checkNotNull(this->undoLogSerial);

	int size = sizeof(uint32_t);

	size += this->sessionId->binarySize();
	size += this->undoLogSerial->binarySize();

	return size;
}

void UndoLogsIdkey::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->sessionId);
	BinaryUtils::checkNotNull(this->undoLogSerial);

	out->putInt(UndoLogsKeyFactory::UNDO_LOGS_ID_KEY);
	this->sessionId->toBinary(out);
	this->undoLogSerial->toBinary(out);
}

UndoLogsIdkey* UndoLogsIdkey::fromBinary(ByteBuffer *in) {
	UndoLogsIdkey* key = new UndoLogsIdkey();

	key->sessionId = CdbDatabaseSessionId::createFromBinary(in);
	BinaryUtils::checkNotNull(key->sessionId);

	key->undoLogSerial = CdbOid::fromBinary(in);
	BinaryUtils::checkNotNull(key->undoLogSerial);

	return key;
}

int UndoLogsIdkey::compareTo(const AbstractBtreeKey *key) const noexcept {
	if(key->isInfinity()){
		return -1;
	}
	else if(key->isNull()){
		return 1;
	}

	static CdbOid::ValueCompare cmp;

	const UndoLogsIdkey* other = dynamic_cast<const UndoLogsIdkey*>(key);

	int diff = this->sessionId->compareTo(other->sessionId);
	if(diff != 0){
		return diff;
	}

	return cmp(this->undoLogSerial, other->undoLogSerial);
}

AbstractBtreeKey* UndoLogsIdkey::clone() const noexcept {
	return new UndoLogsIdkey(*this);
}

void UndoLogsIdkey::setSessionId(const CdbDatabaseSessionId *id) noexcept {
	delete this->sessionId;
	this->sessionId = dynamic_cast<CdbDatabaseSessionId*>(id->copyData());
}

void UndoLogsIdkey::setSerial(uint64_t serial) noexcept {
	delete this->undoLogSerial;
	this->undoLogSerial = new CdbOid(serial);
}

} /* namespace codablecash */
