/*
 * InstanceDependencyContext.h
 *
 *  Created on: Nov 21, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_INSTANCE_INSTANCEDEPENDENCYCONTEXT_H_
#define SMARTCONTRACT_INSTANCE_INSTANCEDEPENDENCYCONTEXT_H_

#include "base/ArrayList.h"

namespace alinous {
class CompilationUnit;
class VirtualMachine;
}
using namespace alinous;

namespace codablecash {

class AbstractExecutableModuleInstance;
class ModuleInstanceClassLoader;

class InstanceDependencyContext {
public:
	InstanceDependencyContext();
	virtual ~InstanceDependencyContext();

	void setModuleInstance(AbstractExecutableModuleInstance* inst) noexcept;

	AbstractExecutableModuleInstance* getModuleInstance() const noexcept {
		return this->moduleInstance;
	}

	void setVm(VirtualMachine* vm);
	ModuleInstanceClassLoader* getClassLoader() const noexcept {
		return this->clazzLoader;
	}

private:
	AbstractExecutableModuleInstance* moduleInstance;
	ModuleInstanceClassLoader* clazzLoader;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_INSTANCE_INSTANCEDEPENDENCYCONTEXT_H_ */
