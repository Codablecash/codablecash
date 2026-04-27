/*
 * SmartcontractExecResult.h
 *
 *  Created on: Dec 2, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECRESULT_H_
#define SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECRESULT_H_

namespace alinous {
class AbstractExtObject;
}
using namespace alinous;

namespace codablecash {

class SmartcontractExecResult {
public:
	SmartcontractExecResult();
	virtual ~SmartcontractExecResult();

	void setReturnedValue(AbstractExtObject* value) noexcept;
	AbstractExtObject* getReturnedValue() const noexcept {
		return this->returnedValue;
	}

private:
	AbstractExtObject* returnedValue;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_SMARTCONTRACTEXECRESULT_H_ */
