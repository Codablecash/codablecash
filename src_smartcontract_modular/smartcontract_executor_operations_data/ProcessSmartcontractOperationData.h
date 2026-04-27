/*
 * ProcessSmartcontractOperationData.h
 *
 *  Created on: Jan 28, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_OPERATIONS_DATA_PROCESSSMARTCONTRACTOPERATIONDATA_H_
#define SMARTCONTRACT_EXECUTOR_OPERATIONS_DATA_PROCESSSMARTCONTRACTOPERATIONDATA_H_

#include "smartcontract_executor_operations_data/AbstractSmartcontractOperationData.h"

namespace alinous {
class AbstractFunctionExtArguments;
}

namespace codablecash {

class ProcessSmartcontractOperationData : public AbstractSmartcontractOperationData {
public:
	ProcessSmartcontractOperationData(const ProcessSmartcontractOperationData& inst);
	ProcessSmartcontractOperationData();
	virtual ~ProcessSmartcontractOperationData();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	void setModule(const UnicodeString* module);
	void setMethodName(const UnicodeString* methodName);
	void addFunctionArgument(const AbstractFunctionExtArguments* arg);

	const UnicodeString* getModule() const noexcept {
		return this->module;
	}
	const UnicodeString* getMethodName() const noexcept {
		return this->methodName;
	}
	ArrayList<AbstractFunctionExtArguments>* getAruments() const noexcept {
		return this->args;
	}

private:
	UnicodeString* module;
	UnicodeString* methodName;
	ArrayList<AbstractFunctionExtArguments> *args;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_OPERATIONS_DATA_PROCESSSMARTCONTRACTOPERATIONDATA_H_ */
