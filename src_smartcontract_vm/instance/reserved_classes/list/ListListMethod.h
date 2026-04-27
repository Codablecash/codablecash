/*
 * ListListMethod.h
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_LIST_LISTLISTMETHOD_H_
#define INSTANCE_RESERVED_CLASSES_LIST_LISTLISTMETHOD_H_

#include "instance/reserved_generics/AbstractGenericReservedMethodDeclare.h"

namespace alinous {

class ListListMethod : public AbstractGenericReservedMethodDeclare {
public:
	static const UnicodeString METHOD_NAME;

	ListListMethod();
	virtual ~ListListMethod();

	virtual void init(VirtualMachine* vm);
	virtual void interpret(FunctionArguments* args, VirtualMachine* vm);
	virtual const UnicodeString* toString();

	virtual MethodDeclare* generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const;
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_LIST_LISTLISTMETHOD_H_ */
