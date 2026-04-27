/*
 * ModuleInstanceClassLoader.h
 *
 *  Created on: Nov 22, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_INSTANCE_MODULEINSTANCECLASSLOADER_H_
#define SMARTCONTRACT_INSTANCE_MODULEINSTANCECLASSLOADER_H_

#include "base/ArrayList.h"
#include "base/HashMap.h"

#include "vm/IInitializeCompilantUnitProvidor.h"

namespace alinous {
class CompilationUnit;
class VirtualMachine;
class ClassDeclare;
class AnalyzeContext;
}
using namespace alinous;


namespace codablecash {

class ModuleInstanceClassLoader : public IInitializeCompilantUnitProvidor {
public:
	ModuleInstanceClassLoader();
	virtual ~ModuleInstanceClassLoader();

	void setVm(VirtualMachine* vm);

	void loadClass(const UnicodeString* fqn);
	ClassDeclare* getClassDeclare(const UnicodeString* fqn) const;

	void preAnalyze(AnalyzeContext* actx);
	void analyzeType(AnalyzeContext* actx);
	void analyze(AnalyzeContext* actx);

	virtual void initCompilantUnits(VirtualMachine *vm);

private:
	void addCompilationUnit(const UnicodeString *fqn, CompilationUnit* unit) noexcept;

private:
	ArrayList<CompilationUnit>* progs;
	HashMap<UnicodeString, CompilationUnit>* classFqnMap;
	HashMap<UnicodeString, CompilationUnit>* filePathmap;

	VirtualMachine* vm;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_INSTANCE_MODULEINSTANCECLASSLOADER_H_ */
