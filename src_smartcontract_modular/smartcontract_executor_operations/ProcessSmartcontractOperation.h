/*
 * ProcessSmartcontractOperation.h
 *
 *  Created on: Jan 21, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_OPERATIONS_PROCESSSMARTCONTRACTOPERATION_H_
#define SMARTCONTRACT_EXECUTOR_OPERATIONS_PROCESSSMARTCONTRACTOPERATION_H_

#include "smartcontract_executor_operations/AbstractSmartcontractOperation.h"

#include "base/ArrayList.h"

namespace alinous {
class AbstractFunctionExtArguments;
}
using namespace alinous;

namespace codablecash {

class CdbDatabaseSessionId;
class SmartcontractInstanceAddress;


class ProcessSmartcontractOperation : public AbstractSmartcontractOperation {
public:
	ProcessSmartcontractOperation();
	virtual ~ProcessSmartcontractOperation();

	void setInstanceAddress(const SmartcontractInstanceAddress *address);

	void setModule(const UnicodeString* module);
	void setMethodName(const UnicodeString* methodName);
	void addFunctionArgument(const AbstractFunctionExtArguments* arg);

	virtual void execute(ModularSmartcontractExecutor* executor);

private:
	SmartcontractInstanceAddress *address;

	UnicodeString* module;
	UnicodeString* methodName;
	ArrayList<AbstractFunctionExtArguments> *args;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_OPERATIONS_PROCESSSMARTCONTRACTOPERATION_H_ */
