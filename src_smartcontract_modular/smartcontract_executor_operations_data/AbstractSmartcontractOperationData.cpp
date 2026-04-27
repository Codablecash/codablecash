/*
 * AbstractSmartcontractOperationData.cpp
 *
 *  Created on: Jan 28, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_operations_data/AbstractSmartcontractOperationData.h"
#include "smartcontract_executor_operations_data/CreateSmartcontractInstanceOperationData.h"
#include "smartcontract_executor_operations_data/ProcessSmartcontractOperationData.h"

#include "base_io/ByteBuffer.h"

#include "filestore_block/exceptions.h"


namespace codablecash {

AbstractSmartcontractOperationData::AbstractSmartcontractOperationData(const AbstractSmartcontractOperationData &inst) {
	this->kind = inst.kind;
}

AbstractSmartcontractOperationData::AbstractSmartcontractOperationData(uint8_t kind) {
	this->kind = kind;
}

AbstractSmartcontractOperationData::~AbstractSmartcontractOperationData() {

}

AbstractSmartcontractOperationData* AbstractSmartcontractOperationData::createFromBinary(ByteBuffer *in) {
	AbstractSmartcontractOperationData* data = nullptr;

	uint8_t kind = in->get();
	switch(kind){
	case KIND_CREATE_SMARTCONTRACT:
		data = new CreateSmartcontractInstanceOperationData();
		break;
	case KIND_PROCESS_SMARTCONTRACT:
		data = new ProcessSmartcontractOperationData();
		break;
	default:
		throw new BinaryFormatException(__FILE__, __LINE__);
	}

	data->fromBinary(in);

	return data;
}

} /* namespace codablecash */
