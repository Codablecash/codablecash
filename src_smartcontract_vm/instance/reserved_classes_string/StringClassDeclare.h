/*
 * StringClassDeclare.h
 *
 *  Created on: 2020/03/16
 *      Author: iizuka
 */

#ifndef RESERVED_CLASSES_STRING_STRINGCLASSDECLARE_H_
#define RESERVED_CLASSES_STRING_STRINGCLASSDECLARE_H_

#include "instance/reserved_classes/AbstractReservedClassDeclare.h"

namespace alinous {

class UnicodeString;
class AnalyzedClass;

class StringClassDeclare : public AbstractReservedClassDeclare {
public:
	static UnicodeString NAME;

	static AnalyzedClass* createAnalyzedClass() noexcept;

	static AnalyzedClass* getStaticAnalyzedClass();

	StringClassDeclare();
	virtual ~StringClassDeclare();

	virtual uint16_t getClassType() const noexcept {
		return TYPE_STRING_CLASS;
	}

	virtual ClassDeclare* getBaseClass() const noexcept;
	virtual const UnicodeString* getPackageName() const noexcept;
};

} /* namespace alinous */

#endif /* RESERVED_CLASSES_STRING_STRINGCLASSDECLARE_H_ */
