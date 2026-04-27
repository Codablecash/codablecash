/*
 * SmartcontractContextMover.h
 *
 *  Created on: Mar 26, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCONTEXTMOVER_H_
#define SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCONTEXTMOVER_H_

namespace codablecash {

class SmartcontractExecContextRegistory;
class CdbDatabaseSessionId;
class SmartcontractCallSequence;
class ModularSmartcontractExecutor;
class SmartcontractInstanceAddress;
class ISystemLogger;

class SmartcontractContextMover {
public:
	static constexpr const int MOVE_FORWARD = 1;
	static constexpr const int MOVE_BACKWARD = 2;
	static constexpr const int MOVE_BACK_FORWARD = 3;

	SmartcontractContextMover(ModularSmartcontractExecutor* executor, const SmartcontractInstanceAddress* address, const CdbDatabaseSessionId* trxSrc, const CdbDatabaseSessionId* trxDst);
	virtual ~SmartcontractContextMover();

	void execute(ISystemLogger *logger);

private:
	void moveBackword(const SmartcontractCallSequence* sequence, ISystemLogger *logger);
	void __moveBackword(const SmartcontractCallSequence* sequence, const CdbDatabaseSessionId* __trxDst, ISystemLogger *logger);
	void moveForword(const SmartcontractCallSequence* sequence, ISystemLogger *logger);
	void __moveForword(const SmartcontractCallSequence* sequence, const CdbDatabaseSessionId* __trxSrc, const CdbDatabaseSessionId* __trxDst, ISystemLogger *logger);
	void moveBackAndForword(const SmartcontractCallSequence* sequence, const SmartcontractCallSequence* sequenceDst, ISystemLogger *logger);

private:
	ModularSmartcontractExecutor* executor;
	SmartcontractInstanceAddress* address;
	CdbDatabaseSessionId* trxSrc;
	CdbDatabaseSessionId* trxDst;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_CURSOR_SMARTCONTRACTCONTEXTMOVER_H_ */
