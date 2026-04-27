/*
 * AbstractExceptionClassDeclare.cpp
 *
 *  Created on: 2020/04/18
 *      Author: iizuka
 */

#include "instance/instance_exception_class/AbstractExceptionClassDeclare.h"

#include "engine/sc_analyze/AnalyzedClass.h"

#include "base/UnicodeString.h"


namespace alinous {

const UnicodeString AbstractExceptionClassDeclare::PACKAGE_NAME(L"lang");

AbstractExceptionClassDeclare::AbstractExceptionClassDeclare() : AbstractReservedClassDeclare() {

}

AbstractExceptionClassDeclare::~AbstractExceptionClassDeclare() {

}

const alinous::UnicodeString* AbstractExceptionClassDeclare::getPackageName() const noexcept {
	return &PACKAGE_NAME;
}

} /* namespace alinous */
