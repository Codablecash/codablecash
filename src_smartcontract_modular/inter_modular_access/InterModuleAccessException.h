/*
 * InterModuleAccessException.h
 *
 *  Created on: Dec 6, 2025
 *      Author: iizuka
 */

#ifndef INTER_MODULAR_ACCESS_INTERMODULEACCESSEXCEPTION_H_
#define INTER_MODULAR_ACCESS_INTERMODULEACCESSEXCEPTION_H_

#include "instance/instance_exception_class/AbstractExceptionClassDeclare.h"

namespace alinous {
class UnicodeString;
class AnalyzedClass;
class VirtualMachine;
class CodeElement;
}
using namespace alinous;

namespace codablecash {

class InterModuleAccessException : public AbstractExceptionClassDeclare {
public:
	static const UnicodeString NAME;
	static const UnicodeString FULL_QUALIFIED_NAME;

	static AnalyzedClass* createAnalyzedClass() noexcept;
	static void throwException(const UnicodeString* msg, VirtualMachine* vm, const CodeElement* element) noexcept;

	InterModuleAccessException();
	virtual ~InterModuleAccessException();

	virtual uint16_t getClassType() const noexcept {
		return TYPE_INTER_MODULE_ACCESS_EXCEPTION;
	}

	virtual ClassDeclare* getBaseClass() const noexcept;

	virtual IVmInstanceFactory* getFactory() const noexcept;
};

} /* namespace codablecash */

#endif /* INTER_MODULAR_ACCESS_INTERMODULEACCESSEXCEPTION_H_ */
