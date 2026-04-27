/*
 * SessionContextCursor.cpp
 *
 *  Created on: Mar 22, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_cursor/SessionContextCursor.h"
#include "smartcontract_executor_cursor/SessionContextCursorData.h"

#include "smartcontract_executor/SmartcontractExecContextRegistory.h"
#include "smartcontract_executor/SmartcontractExecContextData.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "base/StackRelease.h"

namespace codablecash {

SessionContextCursor::SessionContextCursor(SmartcontractExecContextRegistory* contextRegistory) {
	this->contextRegistory = contextRegistory;
	this->trxId = nullptr;
	this->data = nullptr;
}

SessionContextCursor::~SessionContextCursor() {
	delete this->trxId;
	delete this->data;
}

void SessionContextCursor::setPosition(const CdbDatabaseSessionId *trxId) {
	delete this->trxId, this->trxId = nullptr;
	delete this->data, this->data = nullptr;

	this->trxId = dynamic_cast<CdbDatabaseSessionId*>(trxId->copyData());

	this->data = this->contextRegistory->getExecContext(this->trxId);
}

void SessionContextCursor::previous() {
	const CdbDatabaseSessionId* lastTrxId = this->data->getLastTrxId();

	CdbDatabaseSessionId* trxId = dynamic_cast<CdbDatabaseSessionId*>(lastTrxId->copyData()); __STP(trxId);

	setPosition(trxId);
}

bool SessionContextCursor::hasPrevious() const noexcept {
	const CdbDatabaseSessionId* lastId = this->data->getLastTrxId();
	bool bl = this->data->isFinalized();

	return bl == false && lastId != nullptr;
}

SessionContextCursorData* SessionContextCursor::getData() const {
	SessionContextCursorData* data = new SessionContextCursorData(); __STP(data);

	data->setSessionId(this->trxId);

	data->setCallSerial(this->data->getCallSerial());
	data->setHeight(this->data->getHeight());
	data->setBlockHead(this->data->getBlockHeaderId());
	data->setOperationData(this->data->getOpData());

	data->setLastSessionId(this->data->getLastTrxId());

	data->setFinalized(this->data->isFinalized());

	return __STP_MV(data);
}

} /* namespace codablecash */
