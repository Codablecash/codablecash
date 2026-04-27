/*
 * ModularSmartcontractExecutor.cpp
 *
 *  Created on: Sep 22, 2025
 *      Author: iizuka
 */

#include "smartcontract_executor/ModularSmartcontractExecutor.h"
#include "smartcontract_executor/SmartcontractExecContextRegistory.h"

#include "smartcontract_executor_index/InstanceIdIndex.h"

#include "smartcontract_executor_index_projectid/ProjectIdIndex.h"

#include "smartcontract_executor_operations/CreateSmartcontractInstanceOperation.h"
#include "smartcontract_executor_operations/ProcessSmartcontractOperation.h"
#include "smartcontract_executor_operations/RegisterSmartcontractProjectOperation.h"

#include "modular_project/ModularSmartcontractProject.h"

#include "modular_project_registory/ModularSmartcontractProjectRegistory.h"
#include "modular_project_registory/SmartcontractProjectData.h"
#include "modular_project_registory/ProjectIdKey.h"

#include "smartcontract_instance/ModularSmartcontractInstance.h"

#include "smartcontract_cache/InstanceSpacesManager.h"
#include "smartcontract_cache/InstanceSpace.h"
#include "smartcontract_cache/ModuleSetupException.h"
#include "smartcontract_cache/InstanceSpaceReleaser.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_io/File.h"

#include "bc/ExceptionThrower.h"


namespace codablecash {

ModularSmartcontractExecutor::ModularSmartcontractExecutor(const File* base) {
	this->baseDir = base->get(DIR_NAME);
	this->projectRegistory = new ModularSmartcontractProjectRegistory(this->baseDir);
	this->address2ProjectIdIndex = new ProjectIdIndex(this->baseDir);

	this->execContextRegistory = new SmartcontractExecContextRegistory(this->baseDir);
	this->instanceIndex = new InstanceIdIndex(this->baseDir);

	this->instanceSpace = new InstanceSpacesManager(this->baseDir, this->execContextRegistory, this->instanceIndex, this->projectRegistory);
}

ModularSmartcontractExecutor::~ModularSmartcontractExecutor() {
	delete this->projectRegistory;
	delete this->address2ProjectIdIndex;
	delete this->execContextRegistory;
	delete this->instanceIndex;
	delete this->baseDir;
	delete this->instanceSpace;
}

void ModularSmartcontractExecutor::createExecutor() {
	this->projectRegistory->createBlankDatabase();
	this->address2ProjectIdIndex->createBlankDatabase();
	this->execContextRegistory->createBlankDatabase();
	this->instanceIndex->createBlankDatabase();
}

void ModularSmartcontractExecutor::open() {
	this->projectRegistory->open();
	this->address2ProjectIdIndex->open();
	this->execContextRegistory->open();
	this->instanceIndex->open();
}

void ModularSmartcontractExecutor::close() noexcept {
	this->projectRegistory->close();
	this->address2ProjectIdIndex->close();
	this->execContextRegistory->close();
	this->instanceIndex->close();
}

void ModularSmartcontractExecutor::registerModularSmartcontractProject(ModularSmartcontractProject *project) {
	project->loadProject();

	ModularSmartcontractInstance* inst = project->toInstance(); __STP(inst);
	inst->loadCompilantUnits(project->getProjectBaseDir());
	bool res = inst->hasCompileError();
	ExceptionThrower<ModuleSetupException>::throwExceptionIfCondition(res == true, L"Compile error.", __FILE__, __LINE__);

	SmartcontractProjectData* data = inst->createData(); __STP(data);
	registerModularSmartcontractProject(data);
}

void ModularSmartcontractExecutor::registerModularSmartcontractProject(SmartcontractProjectData *data) {
	const ProjectIdKey* key = data->getKey();
	this->projectRegistory->put(key, data);
}

void ModularSmartcontractExecutor::registerAddress2ProjectIdIndex(const ProjectIdIndexKey *key, const ProjectIndexData *data) {
	this->address2ProjectIdIndex->add(key, data);
}

SmartcontractProjectData* ModularSmartcontractExecutor::getProject(const SmartcontractProjectId *projectId) {
	ProjectIdKey key;
	key.setProjectId(projectId);

	SmartcontractProjectData* data = this->projectRegistory->findProjectById(&key);
	return data;
}

void ModularSmartcontractExecutor::createInstance(const SmartcontractInstanceAddress *instAddress, const SmartcontractProjectId *projectId) {
	{
		InstanceSpace* space = this->instanceSpace->createInstance(instAddress, projectId); __STP(space);

		// database
		File* instanceRootDir = this->baseDir->get(INSTANCES_DIR_NAME); __STP(instanceRootDir);
		space->setDatabaseDir(instanceRootDir);
		space->createDatabase();
		space->loadDatabase();

		// language
		bool hasError = space->analyze();
		ExceptionThrower<ModuleSetupException>::throwExceptionIfCondition(hasError == true, L"Analysis error.", __FILE__, __LINE__);

		space->setMainInstance();
		space->createMainInstance();
		hasError = space->interpretInitializer();
		ExceptionThrower<ModuleSetupException>::throwExceptionIfCondition(hasError == true, L"Main object initialize error.", __FILE__, __LINE__);

		this->instanceSpace->registerCache(__STP_MV(space));
	}

	// dependency module objects
	{
		InstanceSpace* space = this->instanceSpace->loadFromCache(instAddress);
		InstanceSpaceReleaser __releaser(space);

		space->invokeModularProxyListnerMethod();
	}
}

InstanceSpace* ModularSmartcontractExecutor::loadFromCache(const SmartcontractInstanceAddress *instAddress) {
	InstanceSpace* space = this->instanceSpace->loadFromCache(instAddress);

	if(space == nullptr){
		// FIXME load if there is no cache
	}

	return space;
}

RegisterSmartcontractProjectOperation* ModularSmartcontractExecutor::makeRegisterSmartcontractProjectOperation(
		const SmartcontractModuleAddress *moduleAddress,
		SmartcontractProjectData *data, CdbDatabaseSessionId* trxId, uint64_t height, BlockHeaderId* blockHeaderId) {
	RegisterSmartcontractProjectOperation* op = new RegisterSmartcontractProjectOperation(); __STP(op);

	op->setProjectData(data);
	op->setModuleAddress(moduleAddress);

	op->setHeaderId(height, blockHeaderId);
	op->setSessionId(trxId);

	return __STP_MV(op);
}

CreateSmartcontractInstanceOperation* ModularSmartcontractExecutor::makeCreateSmartcontractInstanceOperation(
		const SmartcontractInstanceAddress *address, const SmartcontractProjectId* projectId,
		CdbDatabaseSessionId* trxId, uint64_t height, BlockHeaderId* blockHeaderId) {
	CreateSmartcontractInstanceOperation* op = new CreateSmartcontractInstanceOperation(); __STP(op);

	op->setAddress(address);
	op->setProjectId(projectId);

	op->setHeaderId(height, blockHeaderId);
	op->setSessionId(trxId);

	return __STP_MV(op);
}

ProcessSmartcontractOperation* ModularSmartcontractExecutor::makeProcessSmartcontractOperation(
		const SmartcontractInstanceAddress *address, const CdbDatabaseSessionId *sessionId,
		const CdbDatabaseSessionId *lastSessionId,
		const UnicodeString* module, const UnicodeString* methodName,
		ArrayList<AbstractFunctionExtArguments> *args, uint64_t height, const BlockHeaderId* blockHeaderId) {
	ProcessSmartcontractOperation* op = new ProcessSmartcontractOperation(); __STP(op);

	op->setHeaderId(height, blockHeaderId);

	op->setInstanceAddress(address);
	op->setSessionId(sessionId);
	op->setLastTrxId(lastSessionId);

	op->setModule(module);
	op->setMethodName(methodName);

	int maxLoop = args->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractFunctionExtArguments* arg = args->get(i);
		op->addFunctionArgument(arg);
	}

	return __STP_MV(op);
}

void ModularSmartcontractExecutor::addExecContext(const SmartcontractExecContextKey *key, const SmartcontractExecContextData *data) {
	this->execContextRegistory->addExecContext(key, data);
}

void ModularSmartcontractExecutor::addInstanceIdIndex(const InstanceIdIndexKey *key, const InstanceIdIndexData *data) {
	this->instanceIndex->put(key, data);
}

SmartcontractExecContextData* ModularSmartcontractExecutor::getExecContext(const SmartcontractExecContextKey *key) {
	SmartcontractExecContextData* data = this->execContextRegistory->getExecContext(key);
	return data;
}

SmartcontractProjectId* ModularSmartcontractExecutor::getLatestProjectId(const SmartcontractModuleAddress *moduleAddress) {
	SmartcontractProjectId* projectId = this->address2ProjectIdIndex->getLatestProjectId(moduleAddress);
	return projectId;
}

void ModularSmartcontractExecutor::findHeads(const SmartcontractInstanceAddress *address, IInstanceSessionContextListner *listner) {
	this->instanceIndex->findHeads(address, listner);
}

} /* namespace codablecash */
