/*
 * DatabaseExceptionClassDeclare.h
 *
 *  Created on: 2020/05/25
 *      Author: iizuka
 */

#ifndef TRANSACTION_EXCEPTION_DATABASEEXCEPTIONCLASSDECLARE_H_
#define TRANSACTION_EXCEPTION_DATABASEEXCEPTIONCLASSDECLARE_H_

#include "instance/instance_exception_class/AbstractExceptionClassDeclare.h"

namespace alinous {

class DatabaseExceptionClassDeclare : public AbstractExceptionClassDeclare {
public:
	static const UnicodeString NAME;
	static const UnicodeString FULL_QUALIFIED_NAME;

	static AnalyzedClass* createAnalyzedClass() noexcept;
	static void throwException(const UnicodeString* msg, VirtualMachine* vm, const CodeElement* element) noexcept;

	DatabaseExceptionClassDeclare();
	virtual ~DatabaseExceptionClassDeclare();

	virtual uint16_t getClassType() const noexcept {
		return TYPE_DATABASE_EXCEPTION;
	}

	virtual ClassDeclare* getBaseClass() const noexcept;

	virtual IVmInstanceFactory* getFactory() const noexcept;
};

} /* namespace alinous */

#endif /* TRANSACTION_EXCEPTION_DATABASEEXCEPTIONCLASSDECLARE_H_ */
