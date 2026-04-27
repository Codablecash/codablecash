/*
 * InstanceDependencyHandler.cpp
 *
 *  Created on: Nov 21, 2025
 *      Author: iizuka
 */

#include "smartcontract_instance/InstanceDependencyHandler.h"
#include "smartcontract_instance/AbstractExecutableModuleInstance.h"
#include "smartcontract_instance/ModularSmartcontractInstance.h"
#include "smartcontract_instance/LibraryExectableModuleInstance.h"
#include "smartcontract_instance/InstanceDependencyContext.h"
#include "smartcontract_instance/ModuleInstanceClassLoader.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "modular_project/AbstractDependencyConfig.h"
#include "modular_project/ModularSmartcontractVersion.h"
#include "modular_project/ModularConfigException.h"

#include "engine/sc_analyze/AnalyzeContext.h"

#include "bc/ExceptionThrower.h"


namespace codablecash {

InstanceDependencyHandler::InstanceDependencyHandler() {
	this->map = new HashMap<UnicodeString, InstanceDependencyContext>();
	this->list = new ArrayList<InstanceDependencyContext>();
}

InstanceDependencyHandler::~InstanceDependencyHandler() {
	Iterator<UnicodeString>* it = this->map->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* name = it->next();
		InstanceDependencyContext* ctx = this->map->get(name);

		delete ctx;
	}

	delete this->map;

	delete this->list;
}

void InstanceDependencyHandler::registerDependentInstance(AbstractDependencyConfig *config, ModularSmartcontractInstance *parent, AnalyzeContext* actx) {
	const UnicodeString* name = config->getModuleName();
	const ModularSmartcontractVersion* version = config->getVersion();
	bool local = config->isLocal();

	if(local){
		InstanceDependencyContext* context = new InstanceDependencyContext();

		LibraryExectableModuleInstance* inst = parent->getLibraryModuleInstance(name);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(inst == nullptr, L"Module does not exists.", __FILE__, __LINE__);
		context->setModuleInstance(inst);

		VirtualMachine* vm = inst->getVM();
		context->setVm(vm);

		this->map->put(name, context);
		this->list->addElement(context);
	}
}

void InstanceDependencyHandler::importExportedClasses(AnalyzeContext *actx) {
	Iterator<UnicodeString>* it = this->map->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		InstanceDependencyContext* context = this->map->get(key);
		AbstractExecutableModuleInstance* inst = context->getModuleInstance();

		ModuleInstanceClassLoader* loader = context->getClassLoader();

		// "instance" -> "libExport"
		inst->loadLibExportInterfaceUnits(loader);

		// if library instance, load "exportClasses"
		LibraryExectableModuleInstance* libInst = dynamic_cast<LibraryExectableModuleInstance*>(inst);
		if(libInst != nullptr){
			libInst->loadExportClasses(loader);
		}
	}
}

void InstanceDependencyHandler::preAnalyze(AnalyzeContext *actx) {
	Iterator<UnicodeString>* it = this->map->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		InstanceDependencyContext* context = this->map->get(key);

		ModuleInstanceClassLoader* loader = context->getClassLoader();
		loader->preAnalyze(actx);
	}
}

void InstanceDependencyHandler::analyzeType(AnalyzeContext *actx) {
	Iterator<UnicodeString>* it = this->map->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		InstanceDependencyContext* context = this->map->get(key);

		ModuleInstanceClassLoader* loader = context->getClassLoader();
		loader->analyzeType(actx);
	}
}

void InstanceDependencyHandler::analyze(AnalyzeContext *actx) {
	Iterator<UnicodeString>* it = this->map->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		InstanceDependencyContext* context = this->map->get(key);

		ModuleInstanceClassLoader* loader = context->getClassLoader();
		loader->analyze(actx);
	}
}

} /* namespace codablecash */
