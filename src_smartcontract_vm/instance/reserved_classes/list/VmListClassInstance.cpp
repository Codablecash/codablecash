/*
 * VmListClassInstance.cpp
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#include "instance/reserved_classes/list/VmListClassInstance.h"

namespace alinous {

VmListClassInstance::VmListClassInstance(AnalyzedClass* clazz, VirtualMachine* v)
		: AbstractVmReservedInstance(clazz, v) {

}

VmListClassInstance::~VmListClassInstance() {

}

} /* namespace alinous */
