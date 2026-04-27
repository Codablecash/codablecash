/*
 * ProjectIdKey.cpp
 *
 *  Created on: Nov 27, 2025
 *      Author: iizuka
 */

#include "modular_project_registory/ProjectIdKey.h"
#include "modular_project_registory/ProjectIdKeyFactory.h"

#include "modular_project/SmartcontractProjectId.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"


namespace codablecash {

ProjectIdKey::ProjectIdKey() {
	this->projectId = nullptr;
}

ProjectIdKey::~ProjectIdKey() {
	delete this->projectId;
}

int ProjectIdKey::binarySize() const {
	BinaryUtils::checkNotNull(this->projectId);

	int size = sizeof(uint32_t);
	size += this->projectId->binarySize();

	return size;
}

void ProjectIdKey::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->projectId);

	out->putInt(ProjectIdKeyFactory::PROJECT_ID_KEY);
	this->projectId->toBinary(out);
}

ProjectIdKey* ProjectIdKey::fromBinary(ByteBuffer *in) {
	SmartcontractProjectId* id = SmartcontractProjectId::fromBinary(in); __STP(id);

	ProjectIdKey* key = new ProjectIdKey();
	key->setProjectId(id);

	return key;
}

int ProjectIdKey::compareTo(const AbstractBtreeKey *key) const noexcept {
	if(key->isInfinity()){
		return -1;
	}
	else if(key->isNull()){
		return 1;
	}

	const ProjectIdKey* other = dynamic_cast<const ProjectIdKey*>(key);
	assert(other != nullptr);

	return this->projectId->compareTo(other->projectId);
}

AbstractBtreeKey* ProjectIdKey::clone() const noexcept {
	ProjectIdKey* key = new ProjectIdKey();
	key->setProjectId(this->projectId);
	return key;
}

void ProjectIdKey::setProjectId(const SmartcontractProjectId *id) {
	delete this->projectId;
	this->projectId = new SmartcontractProjectId(*id);
}

} /* namespace codablecash */
