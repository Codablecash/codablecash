/*
 * SessionContextCursor.h
 *
 *  Created on: Mar 22, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_CURSOR_SESSIONCONTEXTCURSOR_H_
#define SMARTCONTRACT_EXECUTOR_CURSOR_SESSIONCONTEXTCURSOR_H_

namespace codablecash {

class SmartcontractExecContextRegistory;
class CdbDatabaseSessionId;
class SmartcontractExecContextData;
class SessionContextCursorData;

class SessionContextCursor {
public:
	explicit SessionContextCursor(SmartcontractExecContextRegistory* contextRegistory);
	virtual ~SessionContextCursor();

	void setPosition(const CdbDatabaseSessionId* trxId);

	bool hasPrevious() const noexcept;
	void previous();

	SessionContextCursorData* getData() const;

private:
	SmartcontractExecContextRegistory* contextRegistory;

	CdbDatabaseSessionId* trxId;
	SmartcontractExecContextData* data;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_CURSOR_SESSIONCONTEXTCURSOR_H_ */
