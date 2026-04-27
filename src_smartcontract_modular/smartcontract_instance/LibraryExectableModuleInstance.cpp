/*
 * LibraryExectableModuleInstance.cpp
 *
 *  Created on: Nov 11, 2025
 *      Author: iizuka
 */

#include "smartcontract_instance/LibraryExectableModuleInstance.h"
#include "smartcontract_instance/ModuleInstanceClassLoader.h"

#include "base/UnicodeString.h"

#include "vm/VirtualMachine.h"

#include "bc_base/BinaryUtils.h"

#include "base_io/ByteBuffer.h"


namespace codablecash {

LibraryExectableModuleInstance::LibraryExectableModuleInstance()
		: AbstractExecutableModuleInstance(TYPE_LIBRARY) {
	this->libraryName = nullptr;
	this->exportClasses = new ArrayList<UnicodeString>();
}

LibraryExectableModuleInstance::~LibraryExectableModuleInstance() {
	delete this->libraryName;

	this->exportClasses->deleteElements();
	delete this->exportClasses;
}

void LibraryExectableModuleInstance::setLibraryName(const UnicodeString *str) noexcept {
	delete this->libraryName;
	this->libraryName = new UnicodeString(str);
}

void LibraryExectableModuleInstance::addExportClass(const UnicodeString *clazz) noexcept {
	this->exportClasses->addElement(new UnicodeString(clazz));
}

void LibraryExectableModuleInstance::loadCompilantUnits(const File *projectBaseDir) {
	resetContract();
	parseSourceFolders(projectBaseDir);
}

void LibraryExectableModuleInstance::loadExportClasses(ModuleInstanceClassLoader *loader) {
	SmartContract* contract = this->vm->getSmartContract();

	int maxLoop = this->exportClasses->size();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* fqn = this->exportClasses->get(i);

		loader->loadClass(fqn);
	}
}

int LibraryExectableModuleInstance::binarySize() const {
	BinaryUtils::checkNotNull(this->libraryName);

	int total = AbstractExecutableModuleInstance::binarySize();

	total += BinaryUtils::stringSize(this->libraryName);

	int maxLoop = this->exportClasses->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* fqn = this->exportClasses->get(i);
		total += BinaryUtils::stringSize(fqn);
	}

	return total;
}

void LibraryExectableModuleInstance::toBinary(ByteBuffer *out) const {
	AbstractExecutableModuleInstance::toBinary(out);

	BinaryUtils::putString(out, this->libraryName);

	int maxLoop = this->exportClasses->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* fqn = this->exportClasses->get(i);
		BinaryUtils::putString(out, fqn);
	}
}

void LibraryExectableModuleInstance::fromBinary(ByteBuffer *in) {
	AbstractExecutableModuleInstance::fromBinary(in);

	this->libraryName = BinaryUtils::getString(in);

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* fqn =  BinaryUtils::getString(in);

		this->exportClasses->addElement(fqn);
	}
}

} /* namespace codablecash */
