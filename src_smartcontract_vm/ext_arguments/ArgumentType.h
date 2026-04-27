/*
 * ArgumentType.h
 *
 *  Created on: Mar 11, 2026
 *      Author: iizuka
 */

#ifndef EXT_ARGUMENTS_ARGUMENTTYPE_H_
#define EXT_ARGUMENTS_ARGUMENTTYPE_H_

#include <cstdint>


namespace alinous {

class UnicodeString;
class AnalyzedType;
class ByteBuffer;
class CodeElement;
class TypeResolver;

class ArgumentType {
public:
	ArgumentType(const ArgumentType& inst);
	explicit ArgumentType(const AnalyzedType* atype);
	ArgumentType();
	virtual ~ArgumentType();

	int binarySize() const;
	void toBinary(ByteBuffer* out) const;
	static ArgumentType* createFromBinary(ByteBuffer *in);

	void setType(uint8_t type);
	uint8_t getType() const noexcept {
		return this->type;
	}

	AnalyzedType* toAnalyzedType(TypeResolver* resolver, CodeElement* element) const;

private:
	uint8_t type;
	UnicodeString* fqn; // reference
	int dim;
	UnicodeString* genericsType;
};

} /* namespace alinous */

#endif /* EXT_ARGUMENTS_ARGUMENTTYPE_H_ */
