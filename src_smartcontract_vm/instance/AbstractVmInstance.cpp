/*
 * AbstractVmInstance.cpp
 *
 *  Created on: 2019/05/02
 *      Author: iizuka
 */

#include "instance/AbstractVmInstance.h"

#include "vm/VirtualMachine.h"
#include "vm/memory/VmMemoryManager.h"
#include <cstdint>

namespace alinous {

AbstractVmInstance::AbstractVmInstance(uint8_t type, uint64_t serial) {
	this->type = type;
	this->serial = serial;
}

AbstractVmInstance::~AbstractVmInstance() {
}

uint8_t AbstractVmInstance::getType() const noexcept {
	return this->type;
}

int AbstractVmInstance::hashCode() const noexcept {
	// inst hash code
	uint64_t addr = (uint64_t)serial;
	return (int)(addr % 1024);
}

const VMemList<AbstractReference>* AbstractVmInstance::getReferences() const noexcept {
	return nullptr;
}

bool AbstractVmInstance::isReference() const noexcept {
	return false;
}

void* AbstractVmInstance::operator new(size_t size, VirtualMachine* vm) {
	VmMemoryManager* mem = vm->getMemory();
	uint64_t mallocSize = size + sizeof(VirtualMachine**);

	char* p = mem->malloc(mallocSize);
	VirtualMachine** vmp = (VirtualMachine**)p;
	*vmp = vm;

	return ((char*)p) + sizeof(VirtualMachine**);
}

void AbstractVmInstance::operator delete(void* p, size_t size) {
	char* ptr = ((char*)p) - sizeof(VirtualMachine*);

	VirtualMachine** vm = (VirtualMachine**)ptr;
	VmMemoryManager* mem = (*vm)->getMemory();

	mem->free((char*)ptr);
}

AbstractExtObject* AbstractVmInstance::toClassExtObject(const UnicodeString* name, VTableRegistory* table) {
	return nullptr;
}

bool AbstractVmInstance::isNull() const noexcept {
	return false;
}

bool AbstractVmInstance::isPrimitive() const noexcept {
	return false;
}

} /* namespace alinous */
