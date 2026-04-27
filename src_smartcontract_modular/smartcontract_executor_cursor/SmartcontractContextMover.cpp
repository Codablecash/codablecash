/*
 * SmartcontractContextMover.cpp
 *
 *  Created on: Mar 26, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_cursor/SmartcontractContextMover.h"
#include "smartcontract_executor_cursor/SmartcontractCallSequenceBuilder.h"
#include "smartcontract_executor_cursor/SmartcontractCallSequence.h"
#include "smartcontract_executor_cursor/SessionContextCursorData.h"

#include "smartcontract_executor/SmartcontractExecContextRegistory.h"
#include "smartcontract_executor/ModularSmartcontractExecutor.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "transaction/SmartcontractInstanceAddress.h"

#include "smartcontract_cache/InstanceSpaceReleaser.h"
#include "smartcontract_cache/InstanceSpace.h"

#include "smartcontract_executor_operations_data/ProcessSmartcontractOperationData.h"

#include "smartcontract_executor_operations/ProcessSmartcontractOperation.h"

#include "base/StackRelease.h"
namespace codablecash {

SmartcontractContextMover::SmartcontractContextMover(ModularSmartcontractExecutor* executor, const SmartcontractInstanceAddress* address,
		const CdbDatabaseSessionId* trxSrc, const CdbDatabaseSessionId* trxDst) {
	this->executor = executor;
	this->address = dynamic_cast<SmartcontractInstanceAddress*>(address->copyData());
	this->trxSrc = dynamic_cast<CdbDatabaseSessionId*>(trxSrc->copyData());
	this->trxDst = dynamic_cast<CdbDatabaseSessionId*>(trxDst->copyData());
}

SmartcontractContextMover::~SmartcontractContextMover() {
	delete this->address;
	delete this->trxSrc;
	delete this->trxDst;
}

void SmartcontractContextMover::execute(ISystemLogger *logger) {
	SmartcontractExecContextRegistory* contextRegistory = this->executor->getSmartcontractExecContextRegistory();

	SmartcontractCallSequenceBuilder builder(contextRegistory);
	builder.build(this->trxSrc);
	SmartcontractCallSequenceBuilder builderDst(contextRegistory);
	builderDst.build(this->trxDst);

	const SmartcontractCallSequence* sequence = builder.getSequence();
	const SmartcontractCallSequence* sequenceDst = builderDst.getSequence();

	if(sequence->isSamechain(sequenceDst)){
		uint64_t max = sequence->getMaxCallSerial();
		uint64_t maxDst = sequenceDst->getMaxCallSerial();

		if(max > maxDst){ // backword
			moveBackword(sequence, logger);
		}else{ // forward
			moveForword(sequenceDst, logger);
		}
		return;
	}

	// back and forward
	moveBackAndForword(sequence, sequenceDst, logger);
}

void SmartcontractContextMover::moveBackword(const SmartcontractCallSequence *sequence, ISystemLogger *logger) {
	__moveBackword(sequence, this->trxDst, logger);
}

void SmartcontractContextMover::__moveBackword(const SmartcontractCallSequence *sequence, const CdbDatabaseSessionId *__trxDst, ISystemLogger *logger) {
	sequence->logSequence(logger);

	__trxDst->debugLog(logger);

	InstanceSpace* space = this->executor->loadFromCache(this->address);
	InstanceSpaceReleaser __space(space);

	const CdbDatabaseSessionId* sessionId = space->getDatabaseSessionId();
	sessionId->debugLog(logger);

	while(false == sessionId->equals(__trxDst)){
		space->undoCurrentSession();

		sessionId = space->getDatabaseSessionId();
		sessionId->debugLog(logger);
	}
}

void SmartcontractContextMover::moveForword(const SmartcontractCallSequence *sequence, ISystemLogger *logger) {
	__moveForword(sequence, this->trxSrc, this->trxDst, logger);
}

void SmartcontractContextMover::__moveForword(const SmartcontractCallSequence *sequence, const CdbDatabaseSessionId *__trxSrc, const CdbDatabaseSessionId *__trxDst, ISystemLogger *logger) {
	InstanceSpace* space = this->executor->loadFromCache(this->address);
	InstanceSpaceReleaser __space(space);

	// current position
	int pos = sequence->getPosition(__trxSrc) + 1;

	const CdbDatabaseSessionId* sessionId = space->getDatabaseSessionId();
	while(false == sessionId->equals(__trxDst)){
		SessionContextCursorData* data = sequence->get(pos++);

		const AbstractSmartcontractOperationData* op = data->getOpData();
		const ProcessSmartcontractOperationData* processOp = dynamic_cast<const ProcessSmartcontractOperationData*>(op);
		assert(processOp != nullptr);


		const SmartcontractInstanceAddress* address = this->address;
		const CdbDatabaseSessionId* execTrxId = data->getSessionId();
		const CdbDatabaseSessionId* lastTrxId = data->getLastTrxId();

		const UnicodeString* module = processOp->getModule();
		const UnicodeString* methodName = processOp->getMethodName();
		ArrayList<AbstractFunctionExtArguments>* args = processOp->getAruments();

		uint64_t height = data->getHeight();
		const BlockHeaderId *blockHeaderId = data->getBlockHeaderId();
		ProcessSmartcontractOperation* pop = this->executor->makeProcessSmartcontractOperation(address, execTrxId, lastTrxId, module, methodName,args, height, blockHeaderId); __STP(pop);
		pop->execute(this->executor);

		sessionId = space->getDatabaseSessionId();
	}
}

void SmartcontractContextMover::moveBackAndForword(const SmartcontractCallSequence *sequence, const SmartcontractCallSequence *sequenceDst, ISystemLogger *logger) {
	const SessionContextCursorData* common = sequence->getCommonPoint(sequenceDst);

	CdbDatabaseSessionId* trxIdMid = common->getSessionId();

	__moveBackword(sequence, trxIdMid, logger);
	__moveForword(sequenceDst, trxIdMid, this->trxDst, logger);
}

} /* namespace codablecash */
