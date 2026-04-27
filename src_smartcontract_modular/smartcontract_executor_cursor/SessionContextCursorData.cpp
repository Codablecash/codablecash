/*
 * SessionContextCursorData.cpp
 *
 *  Created on: Mar 23, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_cursor/SessionContextCursorData.h"

#include "trx/session/base/CdbDatabaseSessionId.h"


namespace codablecash {

SessionContextCursorData::SessionContextCursorData(const SessionContextCursorData &inst)
		: SmartcontractExecContextData(inst) {
	this->trxId = inst.trxId != nullptr ? dynamic_cast<CdbDatabaseSessionId*>(inst.trxId->copyData()) : nullptr;
}

SessionContextCursorData::SessionContextCursorData() {
	this->trxId = nullptr;
}

SessionContextCursorData::~SessionContextCursorData() {
	delete this->trxId;
}

IBlockObject* SessionContextCursorData::copyData() const noexcept {
	return new SessionContextCursorData(*this);
}

void SessionContextCursorData::setSessionId(const CdbDatabaseSessionId *trxId) {
	delete this->trxId, this->trxId = nullptr;

	this->trxId = dynamic_cast<CdbDatabaseSessionId*>(trxId->copyData());
}

} /* namespace codablecash */
