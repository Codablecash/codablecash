/*
 * SessionContextCursorData.h
 *
 *  Created on: Mar 23, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_CURSOR_SESSIONCONTEXTCURSORDATA_H_
#define SMARTCONTRACT_EXECUTOR_CURSOR_SESSIONCONTEXTCURSORDATA_H_

#include "smartcontract_executor/SmartcontractExecContextData.h"

namespace codablecash {

class SessionContextCursorData : public SmartcontractExecContextData {
public:
	SessionContextCursorData(const SessionContextCursorData& inst);
	SessionContextCursorData();
	virtual ~SessionContextCursorData();

	virtual IBlockObject* copyData() const noexcept;

	void setSessionId(const CdbDatabaseSessionId* trxId);
	CdbDatabaseSessionId* getSessionId() const noexcept {
		return this->trxId;
	}

private:
	CdbDatabaseSessionId* trxId;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_CURSOR_SESSIONCONTEXTCURSORDATA_H_ */
