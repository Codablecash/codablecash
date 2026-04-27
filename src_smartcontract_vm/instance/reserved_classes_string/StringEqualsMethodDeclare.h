/*
 * StringEqualsMethodDeclare.h
 *
 *  Created on: Dec 23, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_STRING_STRINGEQUALSMETHODDECLARE_H_
#define INSTANCE_RESERVED_CLASSES_STRING_STRINGEQUALSMETHODDECLARE_H_

#include "instance/reserved_classes/AbstractReservedMethodDeclare.h"


namespace alinous {

class StringEqualsMethodDeclare : public AbstractReservedMethodDeclare {
public:
	static const UnicodeString METHOD_NAME;

	StringEqualsMethodDeclare();
	virtual ~StringEqualsMethodDeclare();

	virtual void interpret(FunctionArguments* args, VirtualMachine* vm);
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_STRING_STRINGEQUALSMETHODDECLARE_H_ */
