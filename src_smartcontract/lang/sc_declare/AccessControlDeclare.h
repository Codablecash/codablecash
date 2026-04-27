/*
 * AccessControlDeclare.h
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#ifndef SC_DECLARE_ACCESSCONTROLDECLARE_H_
#define SC_DECLARE_ACCESSCONTROLDECLARE_H_

#include "engine/sc/CodeElement.h"

#include "base/HashMap.h"

namespace alinous {

class AbstractType;

class AccessControlDeclare : public CodeElement {
public:
	static const constexpr char PUBLIC{1};
	static const constexpr char PROTECTED{2};
	static const constexpr char PRIVATE{3};

	explicit AccessControlDeclare(char accessCtrl);
	AccessControlDeclare();
	virtual ~AccessControlDeclare();

	void setCtrl(char ctrl) noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	AccessControlDeclare* generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const;

private:
	char accessCtrl;
};

} /* namespace alinous */

#endif /* SC_DECLARE_ACCESSCONTROLDECLARE_H_ */
