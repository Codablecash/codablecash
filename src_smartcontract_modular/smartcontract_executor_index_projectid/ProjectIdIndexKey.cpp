/*
 * ProjectIdIndexKey.cpp
 *
 *  Created on: Jan 29, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_index_projectid/ProjectIdIndexKey.h"
#include "smartcontract_executor_index_projectid/ProjectIdIndexKeyFactory.h"

#include "transaction/SmartcontractModuleAddress.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"

#include "btreekey/NullKey.h"

namespace codablecash {

ProjectIdIndexKey::ProjectIdIndexKey() {
	this->moduleAddress = nullptr;
	this->version = new NullKey();
}

ProjectIdIndexKey::ProjectIdIndexKey(const SmartcontractModuleAddress *moduleAddress, const AbstractBtreeKey* version) {
	this->moduleAddress = moduleAddress != nullptr ? dynamic_cast<SmartcontractModuleAddress*>(moduleAddress->copyData()) : nullptr;
	this->version = dynamic_cast<AbstractBtreeKey*>(version->copyData());
}

ProjectIdIndexKey::~ProjectIdIndexKey() {
	delete this->moduleAddress;
	delete this->version;
}

int ProjectIdIndexKey::binarySize() const {
	BinaryUtils::checkNotNull(this->moduleAddress);
	BinaryUtils::checkNotNull(this->version);

	int size = sizeof(uint32_t);
	size += this->moduleAddress->binarySize();
	size += this->version->binarySize();

	return size;
}

void ProjectIdIndexKey::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->moduleAddress);
	BinaryUtils::checkNotNull(this->version);

	out->putInt(ProjectIdIndexKeyFactory::PROJECT_ID_INDEX_KEY);
	this->moduleAddress->toBinary(out);
	this->version->toBinary(out);
}

ProjectIdIndexKey* ProjectIdIndexKey::fromBinary(ByteBuffer *in) {
	AbstractAddress* address = SmartcontractModuleAddress::createFromBinary(in); __STP(address);
	SmartcontractModuleAddress* contractAddress = dynamic_cast<SmartcontractModuleAddress*>(address);
	BinaryUtils::checkNotNull(contractAddress);

	ProjectIdIndexKeyFactory factory;
	uint32_t keyType = in->getInt();
	AbstractBtreeKey* abkey = factory.fromBinary(keyType, in); __STP(abkey);

	return new ProjectIdIndexKey(contractAddress, abkey);
}

int ProjectIdIndexKey::compareTo(const AbstractBtreeKey *key) const noexcept {
	if(key->isInfinity()){
		return -1;
	}
	else if(key->isNull()){
		return 1;
	}

	const ProjectIdIndexKey* other = dynamic_cast<const ProjectIdIndexKey*>(key);
	int cmp = this->moduleAddress->compareTo(other->moduleAddress);

	return cmp != 0 ? cmp : this->version->compareTo(other->version);
}

AbstractBtreeKey* ProjectIdIndexKey::clone() const noexcept {
	return new ProjectIdIndexKey(this->moduleAddress, this->version);
}


void ProjectIdIndexKey::setModuleAddress(const SmartcontractModuleAddress *moduleAddress) {
	delete this->moduleAddress;
	this->moduleAddress = dynamic_cast<SmartcontractModuleAddress*>(moduleAddress->copyData());
}

void ProjectIdIndexKey::setVersion(const AbstractBtreeKey *v) noexcept {
	delete this->version;
	this->version = dynamic_cast<AbstractBtreeKey*>(v->copyData());
}


} /* namespace codablecash */
