/*
 * ProcessSmartcontractOperation.cpp
 *
 *  Created on: Jan 21, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_operations/ProcessSmartcontractOperation.h"
#include "smartcontract_executor_operations/ModuleNotFoundException.h"

#include "smartcontract_executor/ModularSmartcontractExecutor.h"
#include "smartcontract_executor/SmartcontractExecResult.h"

#include "smartcontract_cache/InstanceSpace.h"
#include "smartcontract_cache/InstanceSpaceReleaser.h"

#include "transaction/SmartcontractInstanceAddress.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "ext_arguments/AbstractFunctionExtArguments.h"

#include "bc/ExceptionThrower.h"

#include "base/StackRelease.h"

#include "smartcontract_executor_index/InstanceIdIndexKey.h"
#include "smartcontract_executor_index/InstanceSessionContext.h"
#include "smartcontract_executor_index/InstanceIdIndexData.h"

#include "smartcontract_executor/SmartcontractExecContextKey.h"
#include "smartcontract_executor/SmartcontractExecContextData.h"

#include "smartcontract_executor_operations_data/ProcessSmartcontractOperationData.h"

#include "smartcontract_executor_cursor/SmartcontractContextMover.h"

namespace codablecash {

ProcessSmartcontractOperation::ProcessSmartcontractOperation() {
	this->address = nullptr;
	this->module = nullptr;
	this->methodName = nullptr;
	this->args = new ArrayList<AbstractFunctionExtArguments>();
}

ProcessSmartcontractOperation::~ProcessSmartcontractOperation() {
	delete this->address;

	delete this->module;
	delete this->methodName;
	this->args->deleteElements();
	delete this->args;
}

void ProcessSmartcontractOperation::setInstanceAddress(const SmartcontractInstanceAddress *address) {
	delete this->address;
	this->address = dynamic_cast<SmartcontractInstanceAddress*>(address->copyData());
}

void ProcessSmartcontractOperation::addFunctionArgument(const AbstractFunctionExtArguments *arg) {
	AbstractFunctionExtArguments* cp = dynamic_cast<AbstractFunctionExtArguments*>(arg->copy());
	this->args->addElement(cp);
}

void ProcessSmartcontractOperation::execute(ModularSmartcontractExecutor *executor) {
	InstanceSpace* space = executor->loadFromCache(this->address);
	InstanceSpaceReleaser __space(space);

	// ExceptionThrower
	ExceptionThrower<ModuleNotFoundException>::throwExceptionIfCondition(space == nullptr, L"Tne module is not found.", __FILE__, __LINE__);

	space->resetRootReference();
	space->createMainInstance();
	space->interpretInitializer();

	// find last session id
	const CdbDatabaseSessionId* currentSessionId = space->getDatabaseSessionId();
	if(!currentSessionId->equals(this->lastTrxId)){
		// SmartcontractContextMover mover(executor, this->address, currentSessionId, this->lastTrxId);
		// mover.execute(); //FIXME mover
	}

	// new session
	space->newSession(this->trxId);

	// exe
	SmartcontractExecResult* result = space->invokeMainObjectMethod(this->module, this->methodName, this->args); __STP(result);

	// context
	uint64_t callSerial = 0;
	{
		SmartcontractExecContextKey lastKey;
		lastKey.setTransactionId(this->lastTrxId);
		SmartcontractExecContextData* lastData = executor->getExecContext(&lastKey); __STP(lastData);

		uint64_t lastCallSerial = lastData->getCallSerial();
		callSerial = lastCallSerial + 1;

		// key
		SmartcontractExecContextKey key;
		key.setTransactionId(this->trxId);

		// data
		SmartcontractExecContextData contextData;
		contextData.setCallSerial(callSerial);
		contextData.setHeight(this->height);
		contextData.setBlockHead(this->blockHeaderId);
		contextData.setLastSessionId(this->lastTrxId);
		contextData.setFinalized(false);

		ProcessSmartcontractOperationData data;
		{
			data.setModule(this->module);
			data.setMethodName(this->methodName);

			int maxLoop = this->args->size();
			for(int i = 0; i != maxLoop; ++i){
				AbstractFunctionExtArguments* arg = this->args->get(i);
				data.addFunctionArgument(arg);
			}
		}
		contextData.setOperationData(&data);

		executor->addExecContext(&key, &contextData);
	}

	// instance index
	{
		InstanceIdIndexKey key;
		key.setInstanceAddress(this->address);
		key.setHeight(this->height);

		InstanceIdIndexData data;
		{
			InstanceSessionContext* context = new InstanceSessionContext(); __STP(context);
			context->setCallSerial(callSerial);
			context->setTrxId(this->trxId);

			data.addSessionContext(__STP_MV(context));
		}

		executor->addInstanceIdIndex(&key, &data);
	}
}

void ProcessSmartcontractOperation::setModule(const UnicodeString *module) {
	delete this->module, this->module = nullptr;
	this->module = new UnicodeString(module);
}

void ProcessSmartcontractOperation::setMethodName(const UnicodeString *methodName) {
	delete this->methodName, this->methodName = nullptr;
	this->methodName = new UnicodeString(methodName);
}

} /* namespace codablecash */
