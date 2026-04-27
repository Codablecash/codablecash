/*
 * CreateSmartcontractInstance.h
 *
 *  Created on: Jan 21, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_OPERATIONS_CREATESMARTCONTRACTINSTANCEOPERATION_H_
#define SMARTCONTRACT_EXECUTOR_OPERATIONS_CREATESMARTCONTRACTINSTANCEOPERATION_H_

#include "smartcontract_executor_operations/AbstractSmartcontractOperation.h"

namespace codablecash {

class SmartcontractInstanceAddress;
class SmartcontractProjectId;


class CreateSmartcontractInstanceOperation : public AbstractSmartcontractOperation {
public:
	CreateSmartcontractInstanceOperation();
	virtual ~CreateSmartcontractInstanceOperation();

	void setAddress(const SmartcontractInstanceAddress *address);
	void setProjectId(const SmartcontractProjectId* projectId);

	virtual void execute(ModularSmartcontractExecutor* executor);

private:
	SmartcontractInstanceAddress *address;
	SmartcontractProjectId* projectId;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_OPERATIONS_CREATESMARTCONTRACTINSTANCEOPERATION_H_ */
