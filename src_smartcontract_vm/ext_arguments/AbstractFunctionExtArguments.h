/*
 * AbstractFunctionExtArguments.h
 *
 *  Created on: 2019/11/17
 *      Author: iizuka
 */

#ifndef EXT_ARGUMENTS_ABSTRACTFUNCTIONEXTARGUMENTS_H_
#define EXT_ARGUMENTS_ABSTRACTFUNCTIONEXTARGUMENTS_H_

#include <cstdint>

namespace alinous {

class ArgumentType;
class AbstractVmInstance;
class VirtualMachine;
class AbstractReference;
class IAbstractVmInstanceSubstance;
class ByteBuffer;

class AbstractFunctionExtArguments {
public:
	static constexpr const uint8_t ARG_NULL = 1;
	static constexpr const uint8_t ARG_BOOL = 2;
	static constexpr const uint8_t ARG_NUMERIC = 3;
	static constexpr const uint8_t ARG_STRING = 4;

	AbstractFunctionExtArguments() = delete;
	explicit AbstractFunctionExtArguments(uint8_t type);
	virtual ~AbstractFunctionExtArguments();

	virtual ArgumentType* getType() const noexcept = 0;
	virtual AbstractVmInstance* interpret(VirtualMachine* vm) = 0;

	virtual AbstractFunctionExtArguments* copy() const = 0;

	virtual int binarySize() const = 0;
	virtual void toBinary(ByteBuffer* out) const = 0;
	virtual void fromBinary(ByteBuffer* in) = 0;

	static AbstractFunctionExtArguments* createFromBinary(ByteBuffer* in);

protected:
	uint8_t argType;
};

} /* namespace alinous */

#endif /* EXT_ARGUMENTS_ABSTRACTFUNCTIONEXTARGUMENTS_H_ */
