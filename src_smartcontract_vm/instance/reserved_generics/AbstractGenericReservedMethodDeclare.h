/*
 * AbstractGenericReservedMethodDeclare.h
 *
 *  Created on: Nov 7, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_GENERICS_ABSTRACTGENERICRESERVEDMETHODDECLARE_H_
#define INSTANCE_RESERVED_GENERICS_ABSTRACTGENERICRESERVEDMETHODDECLARE_H_

#include "instance/reserved_classes/AbstractReservedMethodDeclare.h"

namespace alinous {

class AbstractGenericReservedMethodDeclare : public AbstractReservedMethodDeclare {
public:
	AbstractGenericReservedMethodDeclare(uint32_t methodId);
	virtual ~AbstractGenericReservedMethodDeclare();

	virtual MethodDeclare* doGenerateGenericsImplement(AbstractGenericReservedMethodDeclare* inst, HashMap<UnicodeString, AbstractType> *input) const;
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_GENERICS_ABSTRACTGENERICRESERVEDMETHODDECLARE_H_ */
