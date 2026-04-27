/*
 * StackTraceElement.h
 *
 *  Created on: Dec 6, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_INSTANCE_EXCEPTION_CLASS_STACKTRACEELEMENT_H_
#define INSTANCE_INSTANCE_EXCEPTION_CLASS_STACKTRACEELEMENT_H_

#include "instance/AbstractVmInstance.h"

namespace alinous {
class MethodDeclare;
class AbstractExpression;
}
using namespace alinous;

namespace alinous {

class StackTraceElement : public AbstractVmInstance {
public:
	StackTraceElement(uint64_t serial);
	virtual ~StackTraceElement();

	virtual IAbstractVmInstanceSubstance* getInstance() noexcept;
	virtual int valueCompare(const IAbstractVmInstanceSubstance* right) const noexcept;
	virtual const UnicodeString* toString() const noexcept;

	void setCurrentMethod(MethodDeclare* currentMethod);
	void setEntryPoint(AbstractExpression* entryPoint);

private:
	MethodDeclare* currentMethod;
	AbstractExpression* entryPoint;
};

} /* namespace alinous */

#endif /* INSTANCE_INSTANCE_EXCEPTION_CLASS_STACKTRACEELEMENT_H_ */
