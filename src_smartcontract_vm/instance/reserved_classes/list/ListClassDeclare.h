/*
 * ListClassDeclare.h
 *
 *  Created on: Nov 6, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_LIST_LISTCLASSDECLARE_H_
#define INSTANCE_RESERVED_CLASSES_LIST_LISTCLASSDECLARE_H_

#include "instance/reserved_generics/AbstractReservedGenericsClassDeclare.h"

namespace alinous {

class ListClassDeclare : public AbstractReservedGenericsClassDeclare {
public:
	static const UnicodeString NAME;
	static const UnicodeString PACKAGE;

	ListClassDeclare();
	virtual ~ListClassDeclare();

	static AnalyzedClass* createAnalyzedClass() noexcept;

	virtual uint16_t getGenericsClassType() const noexcept {
		return TYPE_GENERICS_LIST_CLASS;
	}

	virtual const UnicodeString* getPackageName() const noexcept;

	virtual IVmInstanceFactory* getFactory() const noexcept;
	const virtual UnicodeString* getFullQualifiedName() noexcept;

	virtual ClassDeclare* generateGenericsImplement(HashMap<UnicodeString, AbstractType>* input);
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_LIST_LISTCLASSDECLARE_H_ */
