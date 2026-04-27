/*
 * PackageNameDeclare.h
 *
 *  Created on: 2019/01/19
 *      Author: iizuka
 */

#ifndef SC_DECLARE_PACKAGENAMEDECLARE_H_
#define SC_DECLARE_PACKAGENAMEDECLARE_H_

#include "engine/sc/CodeElement.h"

#include "base/ArrayList.h"
#include "base/HashMap.h"

namespace alinous {

class UnicodeString;
class AbstractType;

class PackageNameDeclare : public CodeElement {
public:
	PackageNameDeclare(const PackageNameDeclare& inst);
	PackageNameDeclare();
	virtual ~PackageNameDeclare();

	static PackageNameDeclare* makeFromString(const UnicodeString* str);

	void addSegment(UnicodeString* seg);

	const UnicodeString* getName() noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	PackageNameDeclare* generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const;

private:
	ArrayList<UnicodeString> list;

	UnicodeString* strName;
};

} /* namespace alinous */

#endif /* SC_DECLARE_PACKAGENAMEDECLARE_H_ */
