/*
 * SoftwareVersionKey.cpp
 *
 *  Created on: Jan 31, 2026
 *      Author: iizuka
 */
#include "smartcontract_executor_index_projectid/SmartcontractVersionKey.h"
#include "smartcontract_executor_index_projectid/ProjectIdIndexKeyFactory.h"

#include "bc/SoftwareVersion.h"

#include "base/StackRelease.h"


namespace codablecash {

SmartcontractVersionKey::SmartcontractVersionKey(const SoftwareVersion* value) {
	this->value = new SoftwareVersion(*value);

}

SmartcontractVersionKey::~SmartcontractVersionKey() {
	delete this->value;
}

int SmartcontractVersionKey::binarySize() const {
	int size = sizeof(uint32_t);
	size += this->value->binarySize();

	return size;
}

void SmartcontractVersionKey::toBinary(ByteBuffer *out) const {
	out->putInt(ProjectIdIndexKeyFactory::SMARTCONTRACT_VERSION_KEY);
	this->value->toBinary(out);
}

SmartcontractVersionKey* SmartcontractVersionKey::fromBinary(ByteBuffer *in) {
	SoftwareVersion* ver = SoftwareVersion::createFromBinary(in); __STP(ver);

	return new SmartcontractVersionKey(ver);
}

int SmartcontractVersionKey::compareTo(const AbstractBtreeKey *key) const noexcept {
	if(key->isInfinity()){
		return -1;
	}
	else if(key->isNull()){
		return 1;
	}

	const SmartcontractVersionKey* other = dynamic_cast<const SmartcontractVersionKey*>(key);

	return this->value->compareTo(other->value);
}

AbstractBtreeKey* SmartcontractVersionKey::clone() const noexcept {
	return new SmartcontractVersionKey(this->value);
}

} /* namespace codablecash */
