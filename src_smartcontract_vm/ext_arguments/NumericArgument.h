/*
 * NumericArgument.h
 *
 *  Created on: 2019/11/17
 *      Author: iizuka
 */

#ifndef EXT_ARGUMENTS_NUMERICARGUMENT_H_
#define EXT_ARGUMENTS_NUMERICARGUMENT_H_

#include "ext_arguments/AbstractFunctionExtArguments.h"
#include <cstdint>

namespace alinous {

class NumericArgument : public AbstractFunctionExtArguments {
public:
	NumericArgument();
	NumericArgument(int64_t value, uint8_t atype);
	virtual ~NumericArgument();

	virtual ArgumentType* getType() const noexcept;
	virtual AbstractVmInstance* interpret(VirtualMachine* vm);

	virtual AbstractFunctionExtArguments* copy() const;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

private:
	int64_t data;
	ArgumentType* atype;
};

} /* namespace alinous */

#endif /* EXT_ARGUMENTS_NUMERICARGUMENT_H_ */
