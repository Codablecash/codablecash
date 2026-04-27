/*
 * CreateSmartcontractInstance.cpp
 *
 *  Created on: Jan 21, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_operations/CreateSmartcontractInstanceOperation.h"

#include "smartcontract_executor_operations_data/CreateSmartcontractInstanceOperationData.h"

#include "smartcontract_executor_index/InstanceIdIndexKey.h"
#include "smartcontract_executor_index/InstanceSessionContext.h"

#include "smartcontract_executor/ModularSmartcontractExecutor.h"
#include "smartcontract_executor/SmartcontractExecContextData.h"
#include "smartcontract_executor/SmartcontractExecContextKey.h"

#include "smartcontract_cache/InstanceSpace.h"
#include "smartcontract_cache/InstanceSpaceReleaser.h"

#include "transaction/SmartcontractInstanceAddress.h"

#include "modular_project/SmartcontractProjectId.h"

#include "base/StackRelease.h"

#include "smartcontract_executor_index/InstanceIdIndexData.h"

namespace codablecash {

CreateSmartcontractInstanceOperation::CreateSmartcontractInstanceOperation() {
	this->address = nullptr;
	this->projectId = nullptr;
}

CreateSmartcontractInstanceOperation::~CreateSmartcontractInstanceOperation() {
	delete this->address;
	delete this->projectId;
}

void CreateSmartcontractInstanceOperation::setAddress(const SmartcontractInstanceAddress *address) {
	delete this->address;
	this->address = dynamic_cast<SmartcontractInstanceAddress*>(address->copyData());
}

void CreateSmartcontractInstanceOperation::setProjectId(const SmartcontractProjectId *projectId) {
	delete this->projectId;
	this->projectId = dynamic_cast<SmartcontractProjectId*>(projectId->copyData());
}

void CreateSmartcontractInstanceOperation::execute(ModularSmartcontractExecutor *executor) {
	executor->createInstance(this->address, this->projectId);

	InstanceSpace* space = executor->loadFromCache(address);
	InstanceSpaceReleaser __space(space);

	const SmartcontractProjectId* projectId = space->getProjectId();

	// trxId
	space->newSession(this->trxId);

	// context
	{
		// key
		SmartcontractExecContextKey key;
		key.setTransactionId(this->trxId);

		// data
		SmartcontractExecContextData contextData;
		contextData.setCallSerial(1);
		contextData.setHeight(this->height);
		contextData.setBlockHead(this->blockHeaderId);
		contextData.setLastSessionId(nullptr);
		contextData.setFinalized(false);

		CreateSmartcontractInstanceOperationData data;
		data.setProjectId(projectId);

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
			context->setCallSerial(1);
			context->setTrxId(this->trxId);

			data.addSessionContext(__STP_MV(context));
		}

		executor->addInstanceIdIndex(&key, &data);
	}
}

} /* namespace codablecash */
