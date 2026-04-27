/*
 * ProjectIndexData.cpp
 *
 *  Created on: Jan 29, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_index_projectid/ProjectIndexData.h"

#include "modular_project/SmartcontractProjectId.h"

#include "bc_base/BinaryUtils.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"

namespace codablecash {

ProjectIndexData::ProjectIndexData(const ProjectIndexData &inst) {
	this->projectId = inst.projectId != nullptr ? dynamic_cast<SmartcontractProjectId*>(inst.projectId->copyData()) : nullptr;
}

ProjectIndexData::ProjectIndexData() {
	this->projectId = nullptr;
}

ProjectIndexData::~ProjectIndexData() {
	delete this->projectId;
}

int ProjectIndexData::binarySize() const {
	BinaryUtils::checkNotNull(this->projectId);

	int total = this->projectId->binarySize();
	return total;
}

void ProjectIndexData::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->projectId);

	this->projectId->toBinary(out);
}

ProjectIndexData* ProjectIndexData::createFromBinary(ByteBuffer *in) {
	SmartcontractProjectId* id = SmartcontractProjectId::fromBinary(in); __STP(id);

	ProjectIndexData* data = new ProjectIndexData();
	data->setProjectId(id);
	return data;
}

IBlockObject* ProjectIndexData::copyData() const noexcept {
	return new ProjectIndexData(*this);
}

void ProjectIndexData::setProjectId(const SmartcontractProjectId *id) {
	delete this->projectId;
	this->projectId = dynamic_cast<SmartcontractProjectId*>(id->copyData());
}

} /* namespace codablecash */
