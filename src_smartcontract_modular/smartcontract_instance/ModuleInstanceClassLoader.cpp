/*
 * ModuleInstanceClassLoader.cpp
 *
 *  Created on: Nov 22, 2025
 *      Author: iizuka
 */

#include "smartcontract_instance/ModuleInstanceClassLoader.h"

#include "engine/sc/CompilationUnit.h"
#include "engine/sc/SmartContract.h"

#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/AnalyzeContext.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "vm/VirtualMachine.h"

#include "lang/sc_declare/ClassDeclare.h"

namespace codablecash {

ModuleInstanceClassLoader::ModuleInstanceClassLoader() {
	this->progs = new ArrayList<CompilationUnit>();
	this->classFqnMap = new HashMap<UnicodeString, CompilationUnit>();
	this->filePathmap = new HashMap<UnicodeString, CompilationUnit>();
	this->vm = nullptr;
}

ModuleInstanceClassLoader::~ModuleInstanceClassLoader() {
	this->progs->deleteElements();
	delete this->progs;

	delete this->classFqnMap;
	delete this->filePathmap;
}

void ModuleInstanceClassLoader::setVm(VirtualMachine *vm) {
	this->vm = vm;
}

void ModuleInstanceClassLoader::loadClass(const UnicodeString *fqn) {
	SmartContract* contarct = this->vm->getSmartContract();
	AnalyzeContext* actx = contarct->getAnalyzeContext();

	const ClassDeclare* clazz = contarct->getClassDeclareByFqn(fqn);
	if(clazz != nullptr && !clazz->isReserved()){
		const CompilationUnit* unit = clazz->getCompilationUnit();

		CompilationUnit* newUnit = unit->copy();
		addCompilationUnit(fqn, newUnit);
	}
}

void ModuleInstanceClassLoader::addCompilationUnit(const UnicodeString *fqn, CompilationUnit *unit) noexcept {
	__STP(unit);

	bool addProg = false;
	CompilationUnit* u = this->classFqnMap->get(fqn);
	if(u == nullptr){
		this->classFqnMap->put(fqn, unit);
	}

	const UnicodeString* path = unit->getProjectRelativePath();
	u = this->filePathmap->get(path);
	if(u == nullptr){
		this->filePathmap->put(path, unit);
		addProg = true;
	}

	if(addProg){
		this->progs->addElement(__STP_MV(unit));
	}
}

ClassDeclare* ModuleInstanceClassLoader::getClassDeclare(const UnicodeString *fqn) const {
	UnicodeString* className = TypeResolver::getClassName(fqn); __STP(className);

	CompilationUnit* unit = this->classFqnMap->get(fqn);
	ClassDeclare* dec = unit->getClassDeclare(className);

	return dec;
}

void ModuleInstanceClassLoader::preAnalyze(AnalyzeContext *actx) {
	int maxLoop = this->progs->size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->progs->get(i);
		unit->preAnalyze(actx);
	}
}

void ModuleInstanceClassLoader::analyzeType(AnalyzeContext *actx) {
	int maxLoop = this->progs->size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->progs->get(i);
		unit->analyzeType(actx);
	}
}

void ModuleInstanceClassLoader::analyze(AnalyzeContext *actx) {
	int maxLoop = this->progs->size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->progs->get(i);
		unit->analyze(actx);
	}
}

void ModuleInstanceClassLoader::initCompilantUnits(VirtualMachine *vm) {
	int maxLoop = this->progs->size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->progs->get(i);
		unit->init(vm);
	}
}

} /* namespace codablecash */
