/*
 * RegisterSmartcontractProjectOperation.h
 *
 *  Created on: Jan 25, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_OPERATIONS_REGISTERSMARTCONTRACTPROJECTOPERATION_H_
#define SMARTCONTRACT_EXECUTOR_OPERATIONS_REGISTERSMARTCONTRACTPROJECTOPERATION_H_

#include "smartcontract_executor_operations/AbstractSmartcontractOperation.h"

namespace codablecash {

class SmartcontractModuleAddress;
class SmartcontractProjectData;

class RegisterSmartcontractProjectOperation : public AbstractSmartcontractOperation {
public:
	RegisterSmartcontractProjectOperation();
	virtual ~RegisterSmartcontractProjectOperation();

	void setProjectData(const SmartcontractProjectData *data);
	void setModuleAddress(const SmartcontractModuleAddress* moduleAddress);

	virtual void execute(ModularSmartcontractExecutor* executor);

private:
	SmartcontractModuleAddress* moduleAddress;
	SmartcontractProjectData *data;
};

} /* namespace alinous */

#endif /* SMARTCONTRACT_EXECUTOR_OPERATIONS_REGISTERSMARTCONTRACTPROJECTOPERATION_H_ */
