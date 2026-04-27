/*
 * StringClassDeclare.cpp
 *
 *  Created on: 2020/03/16
 *      Author: iizuka
 */

#include "instance/reserved_classes_string/StringClassDeclare.h"
#include "instance/reserved_classes_string/StringEqualsMethodDeclare.h"

#include "base/UnicodeString.h"

#include "engine/sc_analyze/AnalyzedClass.h"


namespace alinous {

UnicodeString StringClassDeclare::NAME{L"String"};

StringClassDeclare::StringClassDeclare() : AbstractReservedClassDeclare() {
	this->name = new UnicodeString(&NAME);

	addMethod(new StringEqualsMethodDeclare());
}

StringClassDeclare::~StringClassDeclare() {
}

AnalyzedClass* StringClassDeclare::createAnalyzedClass() noexcept {
	StringClassDeclare* classDec = new StringClassDeclare();
	AnalyzedClass* aclass = new AnalyzedClass(classDec);

	return aclass;
}

ClassDeclare* StringClassDeclare::getBaseClass() const noexcept {
	return nullptr;
}

const UnicodeString* StringClassDeclare::getPackageName() const noexcept {
	return nullptr;
}

AnalyzedClass* StringClassDeclare::getStaticAnalyzedClass() {
	static StringClassDeclare classDec;
	static AnalyzedClass aclass(&classDec);

	return &aclass;
}

} /* namespace alinous */
