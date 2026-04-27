/*
 * VmExceptionInstance.h
 *
 *  Created on: 2020/04/14
 *      Author: iizuka
 */

#ifndef INSTANCE_EXCEPTION_CLASS_VMEXCEPTIONINSTANCE_H_
#define INSTANCE_EXCEPTION_CLASS_VMEXCEPTIONINSTANCE_H_

#include "instance/VmClassInstance.h"
#include "instance/IInstanceContainer.h"

#include "instance/instance_parts/VMemList.h"

namespace alinous {

class StackTraceElement;
class VmString;

class VmExceptionInstance : public VmClassInstance {
public:
	VmExceptionInstance(AnalyzedClass* clazz, VirtualMachine* vm);
	virtual ~VmExceptionInstance();

	void setCodeElement(const CodeElement* element) noexcept;
	const CodeElement* getElement() const noexcept;

	virtual AbstractExtObject* toClassExtObject(const UnicodeString* name, VTableRegistory* reg);

	void addStacktrace(StackTraceElement* stack);

	void setMessage(const UnicodeString* message, VirtualMachine* vm);
	const VmString* getMessage() const noexcept {
		return this->message;
	}

private:
	const CodeElement* element;
	VmString* message;

	VMemList<StackTraceElement>* stacktrace;

};

} /* namespace alinous */

#endif /* INSTANCE_EXCEPTION_CLASS_VMEXCEPTIONINSTANCE_H_ */
