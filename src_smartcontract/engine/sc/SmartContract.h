/*
 * SmartContract.h
 *
 *  Created on: 2018/12/31
 *      Author: iizuka
 */

#ifndef SC_SMARTCONTRACT_H_
#define SC_SMARTCONTRACT_H_

#include "base/ArrayList.h"


namespace alinous {

class File;
class CompilationUnit;
class InputStream;
class VirtualMachine;
class AnalyzeContext;
class UnicodeString;
class VmRootReference;
class VmClassInstance;
class ExtClassObject;
class CompileError;
class ReservedClassRegistory;
class GcManager;
class ClassDeclare;
class IInitializeCompilantUnitProvidor;

class SmartContract {
public:
	SmartContract();
	virtual ~SmartContract();

	CompilationUnit* addCompilationUnit(InputStream* stream, int length, const File* base, File* source);
	CompilationUnit* addCompilationUnit(File* file, const File* base);
	void analyze(VirtualMachine* vm);
	bool hasError() noexcept;

	void initBeforeAnalyze(VirtualMachine* vm);
	void preAnalyze(VirtualMachine* vm);
	void preAnalyzeGenerics(VirtualMachine* vm);
	void analyzeType(VirtualMachine* vm);
	void analyzeMetadata(VirtualMachine* vm);
	void analyzeFinal(VirtualMachine* vm);

	void setMainMethod(const UnicodeString* mainPackage, const UnicodeString* mainClass, const UnicodeString* mainMethod);

	VmRootReference* getRootReference() const noexcept;
	void clearRootReference(VirtualMachine* vm) noexcept;
	void releaseMainInstance(GcManager* gc) noexcept;

	VmClassInstance* createInstance(VirtualMachine* vm, ArrayList<IInitializeCompilantUnitProvidor>* exprogs);
	void initialize(VirtualMachine* vm, ArrayList<IInitializeCompilantUnitProvidor>* exprogs);

	AnalyzeContext* getAnalyzeContext() const noexcept;

	const ArrayList<CompileError>* getCompileErrors() const noexcept;

	int getNumCompilationUnit() const noexcept;
	CompilationUnit* getCompilationUnit(int pos) const noexcept;
	void addCompilationUnit(CompilationUnit* unit);

	ReservedClassRegistory* getReservedClassRegistory() const noexcept;

	bool isInitialized() const noexcept {
		return this->initialized;
	}

	ClassDeclare* getClassDeclareByFqn(const UnicodeString* fqn) const;

	UnicodeString* getMainClassFqn() const noexcept;

private:
	void initializeExprogs(VirtualMachine* vm, ArrayList<IInitializeCompilantUnitProvidor>* exprogs);

private:
	UnicodeString* mainPackage;
	UnicodeString* mainClass;
	UnicodeString* mainMethod;
	ArrayList<CompilationUnit> progs;
	AnalyzeContext* actx;

	VmRootReference* rootReference;
	bool initialized;

	ArrayList<CompileError> compileErrorList;
	ReservedClassRegistory* reservedClassRegistory;
};

} /* namespace alinous */

#endif /* SC_SMARTCONTRACT_H_ */
