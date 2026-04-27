/*
 * AbstractVmReservedInstance.cpp
 *
 *  Created on: Nov 3, 2025
 *      Author: iizuka
 */

#include "instance/reserved_classes/AbstractVmReservedInstance.h"

#include "instance/VmInstanceTypesConst.h"

#include "instance/instance_ref/ObjectReference.h"

namespace alinous {

AbstractVmReservedInstance::AbstractVmReservedInstance(AnalyzedClass* clazz, VirtualMachine* vm)
		: VmClassInstance(VmInstanceTypesConst::INST_OBJ, clazz, vm) {

}

AbstractVmReservedInstance::~AbstractVmReservedInstance() {

}


} /* namespace alinous */
