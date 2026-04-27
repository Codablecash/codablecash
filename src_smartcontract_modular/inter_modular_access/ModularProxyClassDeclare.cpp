/*
 * ModularProxyClassDeclare.cpp
 *
 *  Created on: Dec 3, 2025
 *      Author: iizuka
 */

#include "inter_modular_access/ModularProxyClassDeclare.h"

#include "engine/sc_analyze/IVmInstanceFactory.h"
#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/PackageSpace.h"
#include "engine/sc_analyze/AnalyzeContext.h"

#include "engine/sc/CompilationUnit.h"

#include "base/StackRelease.h"

#include "engine/sc_analyze/ValidationError.h"

namespace codablecash {

ModularProxyClassDeclare::ModularProxyClassDeclare() {
	this->factory = nullptr;
	this->dctx = nullptr;
}

ModularProxyClassDeclare::~ModularProxyClassDeclare() {
	this->factory = nullptr;
	this->dctx = nullptr;
}

IVmInstanceFactory* ModularProxyClassDeclare::getFactory() const noexcept {
	return this->factory;
}

void ModularProxyClassDeclare::setFactory(IVmInstanceFactory *factory) {
	this->factory = factory;
}

void ModularProxyClassDeclare::setDependencyContext(InstanceDependencyContext *dctx) noexcept {
	this->dctx = dctx;
}

const UnicodeString* ModularProxyClassDeclare::getPackageFromIf(ClassDeclare *ifdec) {
	CompilationUnit* originalUnit = ifdec->getCompilationUnit();

	const UnicodeString* pname = originalUnit->getPackageName();

	return pname;
}

UnicodeString* ModularProxyClassDeclare::getNameFromInterface(const UnicodeString *mainFqn, const UnicodeString *ifName) {
	UnicodeString* className = TypeResolver::getClassName(mainFqn);
	UnicodeString* name = TypeResolver::getClassName(ifName); __STP(name);

	className->append(L"Proxy");
	className->append(name);

	return className;
}

void ModularProxyClassDeclare::preAnalyze(AnalyzeContext *actx) {
	CompilationUnit* unit = getCompilationUnit();
	PackageSpace* space = actx->getPackegeSpace(unit->getPackageName());

	AnalyzedClass* dec = space->getClass(this->name);
	if(dec != nullptr){
		actx->addValidationError(ValidationError::CODE_CLASS_ALREADY_EXISTS, this, L"Class {0} is already registered", {this->name});

		return;
	}

	space->addClassDeclare(this);


	AbstractReservedClassDeclare::preAnalyze(actx);
}

} /* namespace codablecash */
