/*
 * ObjectType.h
 *
 *  Created on: 2019/06/09
 *      Author: iizuka
 */

#ifndef SC_DECLARE_TYPES_OBJECTTYPE_H_
#define SC_DECLARE_TYPES_OBJECTTYPE_H_

#include "lang/sc_declare_types/AbstractPrimitiveType.h"

namespace alinous {
class PackageNameDeclare;
class UnicodeString;

class ObjectType : public AbstractType {
protected:
	ObjectType(short kind);
public:
	ObjectType();
	virtual ~ObjectType();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual const UnicodeString* toString() noexcept;
	virtual AbstractType* generateGenericsImplement(HashMap<UnicodeString, AbstractType> *input) const;

	void setPackageName(PackageNameDeclare* packageName) noexcept;
	void setName(UnicodeString* className) noexcept;

	PackageNameDeclare* getPackageNameDeclare() const noexcept;
	virtual const UnicodeString* getClassName() const noexcept;

	virtual void visit(ITypeVisitor* visitor);

protected:
	AbstractType* getAbstractTypeFromMember(HashMap<UnicodeString, AbstractType> *input) const;

protected:
	PackageNameDeclare* packageName;
	UnicodeString* className;

	UnicodeString* str;
};

} /* namespace alinous */

#endif /* SC_DECLARE_TYPES_OBJECTTYPE_H_ */
