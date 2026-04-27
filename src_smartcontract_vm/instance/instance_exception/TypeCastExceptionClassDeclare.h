/*
 * TypeCastException.h
 *
 *  Created on: 2020/04/23
 *      Author: iizuka
 */

#ifndef INSTANCE_EXCEPTION_TYPECASTEXCEPTIONCLASSDECLARE_H_
#define INSTANCE_EXCEPTION_TYPECASTEXCEPTIONCLASSDECLARE_H_

#include "instance/instance_exception_class/AbstractExceptionClassDeclare.h"

namespace alinous {

class TypeCastExceptionClassDeclare : public AbstractExceptionClassDeclare {
public:
	static const UnicodeString NAME;
	static const UnicodeString FULL_QUALIFIED_NAME;
	static AnalyzedClass* createAnalyzedClass() noexcept;
	static void throwException(VirtualMachine* vm, const CodeElement* element) noexcept;
	static void throwException(bool cond, VirtualMachine* vm, const CodeElement* element) noexcept;

	TypeCastExceptionClassDeclare();
	virtual ~TypeCastExceptionClassDeclare();

	virtual uint16_t getClassType() const noexcept {
		return TYPE_TYPE_CAST_EXCEPTION;
	}

	virtual ClassDeclare* getBaseClass() const noexcept;

	virtual IVmInstanceFactory* getFactory() const noexcept;
};

} /* namespace alinous */

#endif /* INSTANCE_EXCEPTION_TYPECASTEXCEPTIONCLASSDECLARE_H_ */
