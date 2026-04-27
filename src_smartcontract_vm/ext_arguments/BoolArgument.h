/*
 * BoolArgument.h
 *
 *  Created on: 2020/03/19
 *      Author: iizuka
 */

#ifndef EXT_ARGUMENTS_BOOLARGUMENT_H_
#define EXT_ARGUMENTS_BOOLARGUMENT_H_

#include "ext_arguments/AbstractFunctionExtArguments.h"

namespace alinous {

class BoolArgument : public AbstractFunctionExtArguments {
public:
	BoolArgument();
	explicit BoolArgument(bool value);
	virtual ~BoolArgument();

	virtual ArgumentType* getType() const noexcept;
	virtual AbstractVmInstance* interpret(VirtualMachine* vm);

	virtual AbstractFunctionExtArguments* copy() const;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

private:
	bool value;
};

} /* namespace alinous */

#endif /* EXT_ARGUMENTS_BOOLARGUMENT_H_ */
