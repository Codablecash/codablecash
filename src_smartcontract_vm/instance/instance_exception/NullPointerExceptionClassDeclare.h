/*
 * NullPointerExceptionClassDeclare.h
 *
 *  Created on: 2020/04/17
 *      Author: iizuka
 */

#ifndef INSTANCE_EXCEPTION_NULLPOINTEREXCEPTIONCLASSDECLARE_H_
#define INSTANCE_EXCEPTION_NULLPOINTEREXCEPTIONCLASSDECLARE_H_

#include "instance/instance_exception_class/AbstractExceptionClassDeclare.h"

namespace alinous {

class NullPointerExceptionClassDeclare : public AbstractExceptionClassDeclare {
public:
	static const UnicodeString NAME;
	static const UnicodeString FULL_QUALIFIED_NAME;

	static AnalyzedClass* createAnalyzedClass() noexcept;
	static void throwException(VirtualMachine* vm, const CodeElement* element) noexcept;
	static void throwException(bool cond, VirtualMachine* vm, const CodeElement* element) noexcept;

	NullPointerExceptionClassDeclare();
	virtual ~NullPointerExceptionClassDeclare();

	virtual uint16_t getClassType() const noexcept {
		return TYPE_NULL_POINTER_EXCEPTION;
	}

	virtual ClassDeclare* getBaseClass() const noexcept;

	virtual IVmInstanceFactory* getFactory() const noexcept;
};

} /* namespace alinous */

#endif /* INSTANCE_EXCEPTION_NULLPOINTEREXCEPTIONCLASSDECLARE_H_ */
