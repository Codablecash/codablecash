/*
 * ModuleProxyInstance.cpp
 *
 *  Created on: Dec 3, 2025
 *      Author: iizuka
 */

#include "inter_modular_access/ModuleProxyInstance.h"

namespace codablecash {

ModuleProxyInstance::ModuleProxyInstance(AnalyzedClass* clazz, VirtualMachine* vm)
		: AbstractVmReservedInstance(clazz, vm) {

}

ModuleProxyInstance::~ModuleProxyInstance() {

}

} /* namespace codablecash */
