/*
 * InstanceDependencyContext.cpp
 *
 *  Created on: Nov 21, 2025
 *      Author: iizuka
 */

#include "smartcontract_instance/InstanceDependencyContext.h"
#include "smartcontract_instance/ModuleInstanceClassLoader.h"

#include "vm/VirtualMachine.h"


namespace codablecash {

InstanceDependencyContext::InstanceDependencyContext() {
	this->moduleInstance = nullptr;
	this->clazzLoader = new ModuleInstanceClassLoader();
}

InstanceDependencyContext::~InstanceDependencyContext() {
	delete this->clazzLoader;
}

void InstanceDependencyContext::setModuleInstance(AbstractExecutableModuleInstance *inst) noexcept {
	this->moduleInstance = inst;
}

void InstanceDependencyContext::setVm(VirtualMachine *vm) {
	this->clazzLoader->setVm(vm);
}

} /* namespace codablecash */
