/*
 * CreateSmartcontractInstanceOperationData.cpp
 *
 *  Created on: Jan 28, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_operations_data/CreateSmartcontractInstanceOperationData.h"

#include "modular_project/SmartcontractProjectId.h"

#include "base_io/ByteBuffer.h"

#include "bc_base/BinaryUtils.h"

namespace codablecash {

CreateSmartcontractInstanceOperationData::CreateSmartcontractInstanceOperationData(const CreateSmartcontractInstanceOperationData &inst)
		: AbstractSmartcontractOperationData(inst) {
	this->projectId = inst.projectId != nullptr ? dynamic_cast<SmartcontractProjectId*>(inst.projectId->copyData()) : nullptr;
}

CreateSmartcontractInstanceOperationData::CreateSmartcontractInstanceOperationData()
		: AbstractSmartcontractOperationData(KIND_CREATE_SMARTCONTRACT) {
	this->projectId = nullptr;
}

CreateSmartcontractInstanceOperationData::~CreateSmartcontractInstanceOperationData() {
	delete this->projectId;
}

int CreateSmartcontractInstanceOperationData::binarySize() const {
	BinaryUtils::checkNotNull(this->projectId);

	int total = sizeof(uint8_t);
	total += this->projectId->binarySize();

	return total;
}

void CreateSmartcontractInstanceOperationData::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->projectId);

	out->put(this->kind);

	this->projectId->toBinary(out);
}

void CreateSmartcontractInstanceOperationData::fromBinary(ByteBuffer *in) {
	this->projectId = SmartcontractProjectId::fromBinary(in);
}

IBlockObject* CreateSmartcontractInstanceOperationData::copyData() const noexcept {
	return new CreateSmartcontractInstanceOperationData(*this);
}

void CreateSmartcontractInstanceOperationData::setProjectId(const SmartcontractProjectId *projectId) noexcept {
	delete this->projectId;
	this->projectId = dynamic_cast<SmartcontractProjectId*>(projectId->copyData());
}

} /* namespace codablecash */
