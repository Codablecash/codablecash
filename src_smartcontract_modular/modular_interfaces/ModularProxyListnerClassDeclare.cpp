/*
 * ModularProxyListnerClassDeclare.cpp
 *
 *  Created on: Dec 3, 2025
 *      Author: iizuka
 */

#include "modular_interfaces/ModularProxyListnerClassDeclare.h"
#include "modular_interfaces/ModuleDetectedMethodDeclare.h"

#include "base/UnicodeString.h"

#include "engine/sc_analyze/AnalyzedClass.h"


namespace codablecash {

const UnicodeString ModularProxyListnerClassDeclare::PACKAGE_NAME(L"lang");
const UnicodeString ModularProxyListnerClassDeclare::NAME(L"ModularProxyListner");

ModularProxyListnerClassDeclare::ModularProxyListnerClassDeclare() {
	this->name = new UnicodeString(&NAME);
	this->interface = true;

	addMethod(new ModuleDetectedMethodDeclare());
}

ModularProxyListnerClassDeclare::~ModularProxyListnerClassDeclare() {

}

AnalyzedClass* ModularProxyListnerClassDeclare::createAnalyzedClass() noexcept {
	ModularProxyListnerClassDeclare* classDec = new ModularProxyListnerClassDeclare();
	AnalyzedClass* aclass = new AnalyzedClass(classDec);

	return aclass;
}

const UnicodeString* ModularProxyListnerClassDeclare::getPackageName() const noexcept {
	return &PACKAGE_NAME;
}

} /* namespace codablecash */
