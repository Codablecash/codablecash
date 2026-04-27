/*
 * AbstractDomInstance.cpp
 *
 *  Created on: 2020/07/01
 *      Author: iizuka
 */

#include "instance/instance_dom/AbstractDomInstance.h"

#include "vm/VirtualMachine.h"

namespace alinous {

AbstractDomInstance::AbstractDomInstance(VirtualMachine* vm, uint8_t type, uint64_t serial) : AbstractVmInstance(type, serial) {

}

AbstractDomInstance::~AbstractDomInstance() {

}

} /* namespace alinous */
