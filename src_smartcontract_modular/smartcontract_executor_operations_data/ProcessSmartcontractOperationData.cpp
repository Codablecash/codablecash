/*
 * ProcessSmartcontractOperationData.cpp
 *
 *  Created on: Jan 28, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_operations_data/ProcessSmartcontractOperationData.h"

#include "base/UnicodeString.h"

#include "base_io/ByteBuffer.h"

#include "ext_arguments/AbstractFunctionExtArguments.h"

#include "bc_base/BinaryUtils.h"

namespace codablecash {

ProcessSmartcontractOperationData::ProcessSmartcontractOperationData(const ProcessSmartcontractOperationData &inst)
		: AbstractSmartcontractOperationData(inst){
	this->module = inst.module != nullptr ? new UnicodeString(inst.module) : nullptr;
	this->methodName = inst.methodName != nullptr ? new UnicodeString(inst.methodName) : nullptr;

	this->args = new ArrayList<AbstractFunctionExtArguments>();

	int maxLoop = inst.args->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractFunctionExtArguments* arg = inst.args->get(i);

		AbstractFunctionExtArguments* cp = arg->copy();
		this->args->addElement(cp);
	}
}

ProcessSmartcontractOperationData::ProcessSmartcontractOperationData()
		: AbstractSmartcontractOperationData(KIND_PROCESS_SMARTCONTRACT){
	this->module = nullptr;
	this->methodName = nullptr;
	this->args = new ArrayList<AbstractFunctionExtArguments>();
}

ProcessSmartcontractOperationData::~ProcessSmartcontractOperationData() {
	delete this->module;
	delete this->methodName;

	this->args->deleteElements();
	delete this->args;
}

int ProcessSmartcontractOperationData::binarySize() const {
	BinaryUtils::checkNotNull(this->module);
	BinaryUtils::checkNotNull(this->methodName);

	int total = sizeof(uint8_t);

	total += BinaryUtils::stringSize(this->module);
	total += BinaryUtils::stringSize(this->methodName);

	int maxLoop = this->args->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		AbstractFunctionExtArguments* arg = this->args->get(i);

		total += arg->binarySize();
	}

	return total;
}

void ProcessSmartcontractOperationData::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->module);
	BinaryUtils::checkNotNull(this->methodName);

	out->put(this->kind);

	BinaryUtils::putString(out, this->module);
	BinaryUtils::putString(out, this->methodName);

	int maxLoop = this->args->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		AbstractFunctionExtArguments* arg = this->args->get(i);

		arg->toBinary(out);
	}
}

void ProcessSmartcontractOperationData::fromBinary(ByteBuffer *in) {
	this->module = BinaryUtils::getString(in);
	this->methodName = BinaryUtils::getString(in);

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		AbstractFunctionExtArguments* arg = AbstractFunctionExtArguments::createFromBinary(in);
		this->args->addElement(arg);
	}
}

IBlockObject* ProcessSmartcontractOperationData::copyData() const noexcept {
	return new ProcessSmartcontractOperationData(*this);
}

void ProcessSmartcontractOperationData::setModule(const UnicodeString *module) {
	this->module = new UnicodeString(module);
}

void ProcessSmartcontractOperationData::setMethodName(const UnicodeString *methodName) {
	this->methodName = new UnicodeString(methodName);
}

void ProcessSmartcontractOperationData::addFunctionArgument(const AbstractFunctionExtArguments *arg) {
	this->args->addElement(arg->copy());
}

} /* namespace codablecash */
