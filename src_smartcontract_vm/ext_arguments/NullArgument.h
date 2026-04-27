/*
 * NullArgument.h
 *
 *  Created on: 2019/11/17
 *      Author: iizuka
 */

#ifndef EXT_ARGUMENTS_NULLARGUMENT_H_
#define EXT_ARGUMENTS_NULLARGUMENT_H_

#include "ext_arguments/AbstractFunctionExtArguments.h"

namespace alinous {

class AnalyzedType;

class NullArgument : public AbstractFunctionExtArguments {
public:
	NullArgument(const NullArgument& inst);
	NullArgument();
	explicit NullArgument(const AnalyzedType* type);
	virtual ~NullArgument();

	virtual ArgumentType* getType() const noexcept;
	virtual AbstractVmInstance* interpret(VirtualMachine* vm);

	virtual AbstractFunctionExtArguments* copy() const;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

private:
	ArgumentType* atype;
};

} /* namespace alinous */

#endif /* EXT_ARGUMENTS_NULLARGUMENT_H_ */
