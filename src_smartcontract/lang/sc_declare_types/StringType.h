/*
 * StringType.h
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#ifndef SC_DECLARE_TYPES_STRINGTYPE_H_
#define SC_DECLARE_TYPES_STRINGTYPE_H_

#include "lang/sc_declare_types/AbstractType.h"

namespace alinous {

class StringType : public AbstractType {
public:
	static const UnicodeString TYPE_NAME;

	StringType();
	virtual ~StringType();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual const UnicodeString* toString() noexcept;
	virtual AbstractType* generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const;

	virtual void visit(ITypeVisitor* visitor);
};

} /* namespace alinous */

#endif /* SC_DECLARE_TYPES_STRINGTYPE_H_ */
