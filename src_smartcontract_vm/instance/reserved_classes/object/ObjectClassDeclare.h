/*
 * ObjectClassDeclare.h
 *
 *  Created on: Nov 2, 2025
 *      Author: iizuka
 */

#ifndef INSTANCE_RESERVED_CLASSES_OBJECT_OBJECTCLASSDECLARE_H_
#define INSTANCE_RESERVED_CLASSES_OBJECT_OBJECTCLASSDECLARE_H_

#include "instance/reserved_classes/AbstractReservedClassDeclare.h"

namespace alinous {

class ObjectClassDeclare : public AbstractReservedClassDeclare {
public:
	static const UnicodeString NAME;
	static const UnicodeString PACKAGE;

	ObjectClassDeclare();
	virtual ~ObjectClassDeclare();

	static AnalyzedClass* createAnalyzedClass() noexcept;

	virtual uint16_t getClassType() const noexcept {
		return AbstractReservedClassDeclare::TYPE_OBJECT;
	}

	virtual const UnicodeString* getName() const noexcept;
	virtual const UnicodeString* getPackageName() const noexcept;

	virtual void init(alinous::VirtualMachine *vm);
	virtual ClassDeclare* getBaseClass() const noexcept;
	virtual IVmInstanceFactory* getFactory() const noexcept;
	const virtual UnicodeString* getFullQualifiedName() noexcept;


private:
	UnicodeString* fqn;
};

} /* namespace alinous */

#endif /* INSTANCE_RESERVED_CLASSES_OBJECT_OBJECTCLASSDECLARE_H_ */
