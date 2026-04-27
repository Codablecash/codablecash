/*
 * InstanceSpacesManager.cpp
 *
 *  Created on: Nov 30, 2025
 *      Author: iizuka
 */

#include "smartcontract_cache/InstanceSpacesManager.h"
#include "smartcontract_cache/MolularSmartcontractCacheManager.h"

#include "smartcontract_executor/SmartcontractExecContextRegistory.h"

#include "smartcontract_executor_index/InstanceIdIndex.h"

#include "modular_project_registory/ModularSmartcontractProjectRegistory.h"
#include "modular_project_registory/ProjectIdKey.h"
#include "modular_project_registory/SmartcontractProjectData.h"

#include "smartcontract_cache/InstanceSpace.h"
#include "smartcontract_cache/ProjectNotFoundException.h"

#include "base/StackRelease.h"

#include "bc/ExceptionThrower.h"

#include "modular_project/SmartcontractProjectId.h"

namespace codablecash {

InstanceSpacesManager::InstanceSpacesManager(const File* baseDir, SmartcontractExecContextRegistory* execContextReg
		, InstanceIdIndex* instanceIdIndex, ModularSmartcontractProjectRegistory* projectReg) {
	this->execContextReg = execContextReg;
	this->instanceIdIndex = instanceIdIndex;
	this->projectReg = projectReg;

	this->cache = new MolularSmartcontractCacheManager(1024);
}

InstanceSpacesManager::~InstanceSpacesManager() {
	this->execContextReg = nullptr;
	this->instanceIdIndex = nullptr;
	this->projectReg = nullptr;

	delete this->cache;
}

InstanceSpace* InstanceSpacesManager::createInstance(const SmartcontractInstanceAddress *instAddress, const SmartcontractProjectId *projectId) {
	ProjectIdKey key;
	key.setProjectId(projectId);

	SmartcontractProjectData* data = this->projectReg->findProjectById(&key); __STP(data);
	ExceptionThrower<ProjectNotFoundException>::throwExceptionIfCondition(data == nullptr, L"", __FILE__, __LINE__);

	ModularSmartcontractInstance* instance = data->toSmartcontractInstance();
	InstanceSpace* space = new InstanceSpace(instAddress, instance); __STP(space);

#ifdef __DEBUG__
	{
		const SmartcontractProjectId* id = space->getProjectId();
		assert(projectId->equals(id));
	}
#endif

	return __STP_MV(space);
}

void InstanceSpacesManager::registerCache(InstanceSpace *space) {
	this->cache->registerInstance(space);
}

InstanceSpace* InstanceSpacesManager::loadFromCache(const SmartcontractInstanceAddress *instAddress) {
	return this->cache->getInstanceSpace(instAddress);
}

} /* namespace codablecash */
