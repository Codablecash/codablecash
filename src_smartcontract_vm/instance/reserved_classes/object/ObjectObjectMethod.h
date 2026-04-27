/*
 * ObjectObject.h
 *
 *  Created on: Nov 3, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_OBJECT_OBJECTOBJECTMETHOD_H_
#define INSTANCE_RESERVED_CLASSES_OBJECT_OBJECTOBJECTMETHOD_H_

#include "instance/reserved_classes/AbstractReservedMethodDeclare.h"

namespace alinous {

class ObjectObjectMethod : public AbstractReservedMethodDeclare {
public:
	static const UnicodeString METHOD_NAME;

	ObjectObjectMethod();
	virtual ~ObjectObjectMethod();

	virtual void init(VirtualMachine* vm);
	virtual void interpret(FunctionArguments* args, VirtualMachine* vm);
	virtual const UnicodeString* toString();
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_OBJECT_OBJECTOBJECTMETHOD_H_ */
