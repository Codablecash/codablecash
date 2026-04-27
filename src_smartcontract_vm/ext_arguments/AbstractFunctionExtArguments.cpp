/*
 * AbstractFunctionExtArguments.cpp
 *
 *  Created on: 2019/11/17
 *      Author: iizuka
 */

#include "ext_arguments/AbstractFunctionExtArguments.h"

#include "instance/AbstractVmInstance.h"
#include "instance/VmInstanceTypesConst.h"

#include "instance/instance_ref/AbstractReference.h"
#include "instance/instance_ref/ObjectReference.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "instance/instance_string/VmStringInstance.h"

#include "base_io/ByteBuffer.h"

#include "ext_arguments/NullArgument.h"
#include "ext_arguments/BoolArgument.h"
#include "ext_arguments/NumericArgument.h"
#include "ext_arguments/StringArgument.h"

namespace alinous {

AbstractFunctionExtArguments::AbstractFunctionExtArguments(uint8_t type) {
	this->argType = type;
}

AbstractFunctionExtArguments::~AbstractFunctionExtArguments() {
}

AbstractFunctionExtArguments* AbstractFunctionExtArguments::createFromBinary(ByteBuffer *in) {
	uint8_t type = in->get();

	AbstractFunctionExtArguments* arg = nullptr;

	switch(type){
	case ARG_NULL:
		arg = new NullArgument();
		break;
	case ARG_BOOL:
		arg = new BoolArgument();
		break;
	case ARG_NUMERIC:
		arg = new NumericArgument();
		break;
	case ARG_STRING:
		arg = new StringArgument();
		break;
	default:
		return nullptr;
	}

	arg->fromBinary(in);

	return arg;
}

} /* namespace alinous */
