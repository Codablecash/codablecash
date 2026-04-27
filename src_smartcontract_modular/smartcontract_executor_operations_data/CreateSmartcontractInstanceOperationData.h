/*
 * CreateSmartcontractInstanceOperationData.h
 *
 *  Created on: Jan 28, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_OPERATIONS_DATA_CREATESMARTCONTRACTINSTANCEOPERATIONDATA_H_
#define SMARTCONTRACT_EXECUTOR_OPERATIONS_DATA_CREATESMARTCONTRACTINSTANCEOPERATIONDATA_H_

#include "smartcontract_executor_operations_data/AbstractSmartcontractOperationData.h"

namespace codablecash {

class SmartcontractProjectId;

class CreateSmartcontractInstanceOperationData : public AbstractSmartcontractOperationData {
public:
	CreateSmartcontractInstanceOperationData(const CreateSmartcontractInstanceOperationData& inst);
	CreateSmartcontractInstanceOperationData();
	virtual ~CreateSmartcontractInstanceOperationData();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	void setProjectId(const SmartcontractProjectId* projectId) noexcept;

private:
	SmartcontractProjectId *projectId;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_OPERATIONS_DATA_CREATESMARTCONTRACTINSTANCEOPERATIONDATA_H_ */
