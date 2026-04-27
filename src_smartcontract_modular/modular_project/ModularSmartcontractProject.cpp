/*
 * ModularSmartcontractProject.cpp
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */

#include "modular_project/ModularSmartcontractProject.h"
#include "modular_project/ExecutableSmartcontractModule.h"
#include "modular_project/LibrarySmartcontractModule.h"
#include "modular_project/ModularProjectConfig.h"
#include "modular_project/ModularConfigException.h"

#include "smartcontract_instance/ModularSmartcontractInstance.h"
#include "smartcontract_instance/ExecutableModuleInstance.h"
#include "smartcontract_instance/LibraryExectableModuleInstance.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "base_io/File.h"

#include "bc/ExceptionThrower.h"


namespace codablecash {

ModularSmartcontractProject::ModularSmartcontractProject(const File* baseDir) {
	this->baseDir = new File(*baseDir);

	this->executableModule = nullptr;
	this->libModules = new ArrayList<LibrarySmartcontractModule>();

	this->config = nullptr;
}

ModularSmartcontractProject::~ModularSmartcontractProject() {
	delete this->baseDir;

	delete this->executableModule;

	this->libModules->deleteElements();
	delete this->libModules;

	delete this->config;
}

void ModularSmartcontractProject::loadProject() {
	if(this->config == nullptr){
		__loadProject();
	}
}

void ModularSmartcontractProject::__loadProject() {
	File* projectConfig = this->baseDir->get(PROJECT_CONFIG_FILE_NAME); __STP(projectConfig);

	this->config = new ModularProjectConfig();
	this->config->load(projectConfig);

	// load executable
	{
		const UnicodeString* executable = this->config->getExecutable();
		if(executable != nullptr){
			loadExecutable(executable);
		}
	}

	// load libraries
	{
		File* libraryPath = this->baseDir->get(MODULE_DIR_NAME); __STP(libraryPath);

		const ArrayList<UnicodeString>* list = this->config->getLibralies();
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			const UnicodeString* lib = list->get(i);
			loadLibrary(libraryPath, lib);
		}
	}
}

void ModularSmartcontractProject::loadExecutable(const UnicodeString *path) {
	File* execPath = this->baseDir->get(path); __STP(execPath);

	ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(!execPath->exists(), L"The executable folder does not exists.", __FILE__, __LINE__);

	this->executableModule = new ExecutableSmartcontractModule(path);
	this->executableModule->load(execPath);
}

void ModularSmartcontractProject::loadLibrary(const File* libraryPath, const UnicodeString *libname) {
	File* libPath = libraryPath->get(libname); __STP(libPath);

	UnicodeString msg(L"The library folder '");
	msg.append(libname);
	msg.append(L"' does not exists.");

	ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(!libPath->exists(), msg.towString(), __FILE__, __LINE__);

	UnicodeString projectRelativePath(MODULE_DIR_NAME);
	projectRelativePath.append(L"/");
	projectRelativePath.append(libname);
	projectRelativePath.append(L"/");

	LibrarySmartcontractModule* libMod = new LibrarySmartcontractModule(&projectRelativePath); __STP(libMod);
	libMod->load(libPath);


	this->libModules->addElement(__STP_MV(libMod));
}

ModularSmartcontractInstance* ModularSmartcontractProject::toInstance() const {
	ModularSmartcontractInstance* instance = new ModularSmartcontractInstance(); __STP(instance);

	instance->setModularProjectConfig(this->config);

	// exec
	{
		AbstractExecutableModuleInstance* inst = this->executableModule->toInstance();
		UnicodeString name(L"exec");
		inst->setName(&name);
		instance->setExecutableModuleInstance(dynamic_cast<ExecutableModuleInstance*>(inst));
	}

	{
		int maxLoop = this->libModules->size();
		for(int i = 0; i != maxLoop; ++i){
			LibrarySmartcontractModule* mod = this->libModules->get(i);

			AbstractExecutableModuleInstance* inst = mod->toInstance();
			LibraryExectableModuleInstance* lib = dynamic_cast<LibraryExectableModuleInstance*>(inst);

			const UnicodeString* name = lib->getLibraryName();
			lib->setName(name);

			if(!instance->libraryExists(name)){
				instance->addLibraryModuleInstance(name, lib);
			}
		}
	}

	return __STP_MV(instance);
}

SmartcontractProjectId* ModularSmartcontractProject::getProjectId() const {
	ModularSmartcontractInstance* instance = toInstance(); __STP(instance);
	instance->loadCompilantUnits(this->baseDir);

	bool res = instance->hasCompileError();
	ExceptionThrower<Exception>::throwExceptionIfCondition(res == true, L"Compile error exists.", __FILE__, __LINE__);

	SmartcontractProjectId* projectId = instance->getProjectId();

	return projectId;
}

} /* namespace codablecash */
