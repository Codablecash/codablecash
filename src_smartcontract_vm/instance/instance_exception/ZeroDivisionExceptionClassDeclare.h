/*
 * ZeroDivisionExceptionClassDeclare.h
 *
 *  Created on: 2020/04/17
 *      Author: iizuka
 */

#ifndef INSTANCE_EXCEPTION_ZERODIVISIONEXCEPTIONCLASSDECLARE_H_
#define INSTANCE_EXCEPTION_ZERODIVISIONEXCEPTIONCLASSDECLARE_H_

#include "instance/instance_exception_class/AbstractExceptionClassDeclare.h"


namespace alinous {

class ZeroDivisionExceptionClassDeclare : public AbstractExceptionClassDeclare {
public:
	static UnicodeString NAME;
	static AnalyzedClass* createAnalyzedClass() noexcept;
	static void throwException(VirtualMachine* vm, const CodeElement* element) noexcept;

	ZeroDivisionExceptionClassDeclare();
	virtual ~ZeroDivisionExceptionClassDeclare();

	virtual uint16_t getClassType() const noexcept {
		return TYPE_ZERO_DIVISION_EXCEPTION;
	}

	virtual ClassDeclare* getBaseClass() const noexcept;

	virtual IVmInstanceFactory* getFactory() const noexcept;

};

} /* namespace alinous */

#endif /* INSTANCE_EXCEPTION_ZERODIVISIONEXCEPTIONCLASSDECLARE_H_ */
