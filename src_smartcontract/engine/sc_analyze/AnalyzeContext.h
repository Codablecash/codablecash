/*
 * AnalyzeContext.h
 *
 *  Created on: 2019/05/22
 *      Author: iizuka
 */

#ifndef SC_ANALYZECONTEXT_H_
#define SC_ANALYZECONTEXT_H_

#include "base/HashMap.h"
#include "base/ArrayList.h"
#include <initializer_list>

namespace alinous {

class VirtualMachine;
class UnicodeString;
class PackageSpace;
class ValidationError;
class CodeElement;
class AnalyzedClass;
class ClassDeclare;
class AnalyzeStackManager;
class TypeResolver;
class AnalyzedClass;
class VTableRegistory;
class AnalyzedType;
class ReservedClassRegistory;
class StaticClassMetadataHolder;
class SmartContract;
class GeneratedGenericsTypeHolder;
class AbstractType;

class AnalyzeContext {
public:
	explicit AnalyzeContext(SmartContract* sc);
	virtual ~AnalyzeContext();

	void setVm(VirtualMachine* vm) noexcept {
		this->vm = vm;
	}
	VirtualMachine* getVm() const noexcept {
		return this->vm;
	}

	PackageSpace* getPackegeSpace(const UnicodeString* spaceName) noexcept;

	void addValidationError(bool condition, int errorCode, CodeElement* element, const wchar_t* msg, std::initializer_list<const UnicodeString*> params) noexcept;
	void addValidationError(int errorCode, CodeElement* element, const UnicodeString* msg, std::initializer_list<const UnicodeString*> params) noexcept;
	void addValidationError(int errorCode, CodeElement* element, const wchar_t* msg, std::initializer_list<const UnicodeString*> params) noexcept;

	bool hasError() noexcept;
	const ArrayList<ValidationError>* getErrors() const noexcept;

	AnalyzedClass* getAnalyzedClass(CodeElement* clazz);
	TypeResolver* getTypeResolver() const noexcept;
	AnalyzeStackManager* getAnalyzeStackManager() const noexcept;

	void setThisClass(AnalyzedClass* thisClass) noexcept;
	AnalyzedClass* getThisClass() const noexcept;
	AnalyzedClass* getLastThisClass() const noexcept;
	void popThisClass() noexcept;

	void analyzeClassInheritance();

	VTableRegistory* getVtableRegistory() const noexcept;

	void setCurrentElement(CodeElement* current) noexcept;
	CodeElement* getCurrentElement() const noexcept;

	void resigterReservedClasses() noexcept;

	// generics
	void detectGenericsType(AbstractType* atype);
	void generateGenericsClasses();
	void preAnalyzeGenerics();
	void analyzeTypeRefGenerics();
	void analyzeGenerics();

	ReservedClassRegistory* getReservedClassRegistory() const noexcept;

	HashMap<UnicodeString, PackageSpace>* getPackageSpaces() const noexcept;

	StaticClassMetadataHolder* getStaticVariableHolder() const noexcept;
	void analyzeStaticVariables() noexcept;

	AnalyzedType* getTmpArrayType() const noexcept {
		return this->tempAtype;
	}
	void setTmpArrayType(AnalyzedType* atype) {
		this->tempAtype = atype;
	}

private:
	void analyzeMembers(PackageSpace* space) noexcept;
	void analyzeMember(AnalyzedClass* cls) noexcept;
	void analyzePackage4StaticVariables(PackageSpace* space) noexcept;

private:
	SmartContract* sc;
	HashMap<UnicodeString, PackageSpace> *packageSpaces;
	ArrayList<ValidationError> verrorList;
	AnalyzeStackManager* stack;
	TypeResolver* typeResolver;
	ArrayList<AnalyzedClass>* thisClasses;
	VTableRegistory* vtableReg;
	StaticClassMetadataHolder* staticVariablesHolder;
	GeneratedGenericsTypeHolder* genericsHolder;

	CodeElement* current;

	// temp value
	VirtualMachine* vm;
	AnalyzedType* tempAtype;
};

} /* namespace alinous */

#endif /* SC_ANALYZECONTEXT_H_ */
