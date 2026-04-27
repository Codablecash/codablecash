/*
 * AbstractGenericReservedMethodDeclare.cpp
 *
 *  Created on: Nov 7, 2025
 *      Author: iizuka
 */

#include "instance/reserved_generics/AbstractGenericReservedMethodDeclare.h"

#include "lang/sc_declare/AccessControlDeclare.h"
#include "lang/sc_declare_types/AbstractType.h"
#include "lang/sc_declare/ArgumentsListDeclare.h"

#include "base/UnicodeString.h"


namespace alinous {

AbstractGenericReservedMethodDeclare::AbstractGenericReservedMethodDeclare(uint32_t methodId)
		: AbstractReservedMethodDeclare(methodId) {

}

AbstractGenericReservedMethodDeclare::~AbstractGenericReservedMethodDeclare() {

}

MethodDeclare* AbstractGenericReservedMethodDeclare::doGenerateGenericsImplement(AbstractGenericReservedMethodDeclare* inst, HashMap<UnicodeString, AbstractType> *input) const {
	inst->copyCodePositions(this);

	AccessControlDeclare* copiedAccess = this->ctrl->generateGenericsImplement(input);
	inst->setAccessControl(copiedAccess);

	if(this->type != nullptr){
		AbstractType* copiedType = this->type->generateGenericsImplement(input);
		inst->setType(copiedType);
	}

	inst->setName(new UnicodeString(this->name));
	inst->setStatic(this->_static);

	ArgumentsListDeclare* copiedArgs = this->args->generateGenericsImplement(input);
	inst->setArguments(copiedArgs);

	return inst;
}

} /* namespace alinous */
