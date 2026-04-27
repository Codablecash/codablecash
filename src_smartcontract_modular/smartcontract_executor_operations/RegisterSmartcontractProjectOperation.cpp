/*
 * RegisterSmartcontractProjectOperation.cpp
 *
 *  Created on: Jan 25, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_operations/RegisterSmartcontractProjectOperation.h"

#include "smartcontract_executor/ModularSmartcontractExecutor.h"

#include "modular_project_registory/SmartcontractProjectData.h"
#include "modular_project_registory/ProjectIdKey.h"

#include "smartcontract_executor_index_projectid/ProjectIdIndexKey.h"
#include "smartcontract_executor_index_projectid/ProjectIndexData.h"

#include "smartcontract_executor_index_projectid/SmartcontractVersionKey.h"

#include "transaction/SmartcontractModuleAddress.h"
#include "transaction/SmartcontractInstanceAddress.h"


namespace codablecash {

RegisterSmartcontractProjectOperation::RegisterSmartcontractProjectOperation() {
	this->moduleAddress = nullptr;
	this->data = nullptr;
}

RegisterSmartcontractProjectOperation::~RegisterSmartcontractProjectOperation() {
	delete this->moduleAddress;
	delete this->data;
}

void RegisterSmartcontractProjectOperation::setProjectData(const SmartcontractProjectData *data) {
	delete this->data;
	this->data = dynamic_cast<SmartcontractProjectData*>(data->copyData());
}

void RegisterSmartcontractProjectOperation::setModuleAddress(const SmartcontractModuleAddress *moduleAddress) {
	delete this->moduleAddress;
	this->moduleAddress = dynamic_cast<SmartcontractModuleAddress*>(moduleAddress->copyData());
}

void RegisterSmartcontractProjectOperation::execute(ModularSmartcontractExecutor *executor) {
	executor->registerModularSmartcontractProject(this->data);

	// register project Id
	ProjectIdIndexKey key;
	{
		key.setModuleAddress(this->moduleAddress);

		const SoftwareVersion* version = this->data->getVersion();

		SmartcontractVersionKey versionKey(version);
		key.setVersion(&versionKey);
	}

	ProjectIndexData indexData;

	const ProjectIdKey* idkey = this->data->getKey();
	const SmartcontractProjectId* projectId = idkey->getProjectId();
	indexData.setProjectId(projectId);

	// call exec
	executor->registerAddress2ProjectIdIndex(&key, &indexData);
}

} /* namespace codablecash */
