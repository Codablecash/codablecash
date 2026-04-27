/*
 * TypeResolver.cpp
 *
 *  Created on: 2019/06/23
 *      Author: iizuka
 */

#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/PackageSpace.h"
#include "engine/sc_analyze/AnalyzedType.h"

#include "engine/sc/CodeElement.h"
#include "engine/sc/CompilationUnit.h"

#include "lang/sc_declare/ImportsDeclare.h"
#include "lang/sc_declare/ImportDeclare.h"
#include "lang/sc_declare/PackageNameDeclare.h"
#include "lang/sc_declare/ClassDeclare.h"
#include "lang/sc_declare/GenericsClassDeclare.h"
#include "lang/sc_declare_types/GenericsObjectType.h"

#include "lang/sc_declare_types/AbstractType.h"
#include "lang/sc_declare_types/ObjectType.h"

#include "base/StackRelease.h"
#include "base/ArrayList.h"

#include "instance/reserved_classes/object/ObjectClassDeclare.h"

#include "instance/reserved_classes_string/StringClassDeclare.h"
namespace alinous {

const UnicodeString TypeResolver::DOT(L".");

const UnicodeString TypeResolver::BOOLEAN(L"boolean");
const UnicodeString TypeResolver::BYTE(L"byte");
const UnicodeString TypeResolver::CHAR(L"char");
const UnicodeString TypeResolver::SHORT(L"short");
const UnicodeString TypeResolver::INT(L"int");
const UnicodeString TypeResolver::LONG(L"long");
const UnicodeString TypeResolver::STRING(L"String");

TypeResolver::TypeResolver(AnalyzeContext* ctx) : ctx(ctx) {

}

TypeResolver::~TypeResolver() {

}

AnalyzedType* TypeResolver::getClassType(CodeElement* element) const {
	ClassDeclare* cls = element->getClassDeclare();
	CompilationUnit* unit = cls->getCompilationUnit();

	const UnicodeString* packageName = unit->getPackageName();
	const UnicodeString* name = cls->getName();

	return findClassType(packageName, name);
}


AnalyzedType* TypeResolver::resolveType(CodeElement* element, AbstractType* type) const {
	AnalyzedType* result = nullptr;

	short kind = type->getKind();
	switch(kind){
	case CodeElement::TYPE_BOOL:
		result = new AnalyzedType(AnalyzedType::TYPE_BOOL);
		break;
	case CodeElement::TYPE_BYTE:
		result = new AnalyzedType(AnalyzedType::TYPE_BYTE);
		break;
	case CodeElement::TYPE_CHAR:
		result = new AnalyzedType(AnalyzedType::TYPE_CHAR);
		break;
	case CodeElement::TYPE_SHORT:
		result = new AnalyzedType(AnalyzedType::TYPE_SHORT);
		break;
	case CodeElement::TYPE_INT:
		result = new AnalyzedType(AnalyzedType::TYPE_INT);
		break;
	case CodeElement::TYPE_LONG:
		result = new AnalyzedType(AnalyzedType::TYPE_LONG);
		break;
	case CodeElement::TYPE_STRING:
		{
			result = new AnalyzedType(AnalyzedType::TYPE_STRING);

			PackageSpace* space = this->ctx->getPackegeSpace(nullptr);
			AnalyzedClass* clazz = space->getClass(&StringClassDeclare::NAME);
			result->setAnalyzedClass(clazz);
		}
		break;
	case CodeElement::TYPE_VOID:
		result = new AnalyzedType(AnalyzedType::TYPE_VOID);
		break;
	case CodeElement::TYPE_OBJECT:
	case CodeElement::TYPE_GENERICS_OBJECT:
		{
			ObjectType* otype = dynamic_cast<ObjectType*>(type);
			result = resolveClassType(element, otype);
			break;
		}
	case CodeElement::TYPE_DOM:
		result = new AnalyzedType(AnalyzedType::TYPE_DOM);
		break;
	default:
		return nullptr;
	}

	if(result != nullptr){
		int dim = type->getDimension();
		result->setDim(dim);
	}

	return result;
}

AnalyzedType* TypeResolver::resolveClassType(CodeElement* element, ObjectType* type) const {
	PackageNameDeclare* pkg = type->getPackageNameDeclare();
	const UnicodeString* name = type->getClassName();

	// generics type
	ClassDeclare* clazz = element->getClassDeclare();
	if(clazz->isGenerics() && pkg == nullptr){
		GenericsClassDeclare* gclazz = dynamic_cast<GenericsClassDeclare*>(clazz);

		const GenericsParameter* result = gclazz->findGenericsType(name);
		if(result != nullptr){
			PackageSpace* space = this->ctx->getPackegeSpace(gclazz->getPackageName());
			AnalyzedClass* dec = space->getClass(gclazz->getName());

			AnalyzedType* atype = new AnalyzedType(AnalyzedType::TYPE_GENERICS_TYPE);
			atype->setAnalyzedClass(dec);
			atype->setGenericsType(name);

			return atype;
		}
	}

	if(pkg != nullptr){
		const UnicodeString* pkgname = pkg->getName();
		return findClassType(pkgname, name);
	}

	return findClassType(element, name);
}

AnalyzedType* TypeResolver::findBasicType(const UnicodeString* name) const {
	AnalyzedType* result = nullptr;

	if(name->equals(BOOLEAN)){
		result = new AnalyzedType(AnalyzedType::TYPE_BOOL);
	}
	else if(name->equals(BYTE)){
		result = new AnalyzedType(AnalyzedType::TYPE_BYTE);
	}
	else if(name->equals(CHAR)){
		result = new AnalyzedType(AnalyzedType::TYPE_CHAR);
	}
	else if(name->equals(SHORT)){
		result = new AnalyzedType(AnalyzedType::TYPE_SHORT);
	}
	else if(name->equals(INT)){
		result = new AnalyzedType(AnalyzedType::TYPE_INT);
	}
	else if(name->equals(LONG)){
		result = new AnalyzedType(AnalyzedType::TYPE_LONG);
	}
	else if(name->equals(STRING)){
		result = new AnalyzedType(AnalyzedType::TYPE_STRING);

		PackageSpace* space = this->ctx->getPackegeSpace(nullptr);
		AnalyzedClass* clazz = space->getClass(&StringClassDeclare::NAME);
		result->setAnalyzedClass(clazz);
	}

	return result;
}

AnalyzedType* TypeResolver::findClassType(const CodeElement* element, const UnicodeString* name) const {
	if(!isFqn(name)){
		CompilationUnit* unit = element->getCompilationUnit();
		ClassDeclare* clazz = element->getClassDeclare();

		// find from generitics params, like T
		if(clazz->isGenerics()) {
			GenericsClassDeclare* gclazz = dynamic_cast<GenericsClassDeclare*>(clazz);

			const GenericsParameter* result = gclazz->findGenericsType(name);
			if(result != nullptr){
				PackageSpace* space = this->ctx->getPackegeSpace(gclazz->getPackageName());
				AnalyzedClass* dec = space->getClass(gclazz->getName());

				AnalyzedType* atype = new AnalyzedType(AnalyzedType::TYPE_GENERICS_TYPE);
				atype->setAnalyzedClass(dec);
				atype->setGenericsType(name);

				return atype;
			}
		}

		// find import section
		ImportsDeclare* imports = unit->getImportDeclare();
		if(imports != nullptr){
			AnalyzedType* atype = findFromImports(name, imports);
			if(atype != nullptr){
				return atype;
			}
		}

		// find same package
		const UnicodeString* packageName = unit->getPackageName();
		AnalyzedType* atype = findClassType(packageName, name);
		if(atype != nullptr){
			return atype;
		}

		// find "lang" package
		if(name->equals(&ObjectClassDeclare::NAME)){
			return findClassType(&ObjectClassDeclare::PACKAGE, &ObjectClassDeclare::NAME);
		}
		atype = findClassType(&ObjectClassDeclare::PACKAGE, name);
		if(atype != nullptr){
			return atype;
		}

		// base class
		atype = findClassType((const UnicodeString*)nullptr, name);
		return atype;
	}

	UnicodeString* pkg = getPackageName(name); __STP(pkg);
	UnicodeString* clazz = getClassName(name); __STP(clazz);

	return findClassType(pkg, clazz);
}

AnalyzedType* TypeResolver::findFromImports(const UnicodeString* name, ImportsDeclare* imports) const {
	const ArrayList<ImportDeclare>* list = imports->getImports();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		ImportDeclare* imp = list->get(i);

		if(imp->hasClassName(name)){
			UnicodeString* pkgName = imp->getPackageName(); __STP(pkgName);
			return findClassType(pkgName, name);
		}
	}

	return nullptr;
}

AnalyzedType* TypeResolver::findClassType(const UnicodeString* packageName, const UnicodeString* name) const {
	PackageSpace* space = this->ctx->getPackegeSpace(packageName);
	if(space == nullptr || space->isEmpty()){
		return nullptr; // error
	}

	AnalyzedClass* clazz = space->getClass(name);
	if(clazz == nullptr){
		return nullptr;
	}

	return new AnalyzedType(clazz);
}

bool alinous::TypeResolver::isFqn(const UnicodeString* name) noexcept {
	return 0 <= name->indexOf(L'.');
}

UnicodeString* TypeResolver::getPackageName(const UnicodeString* name) noexcept {
	int index = name->lastIndexOf(&DOT);
	if(index < 0){
		return nullptr;
	}

	return name->substring(0, index);
}

UnicodeString* TypeResolver::getClassName(const UnicodeString* name) noexcept {
	int index = name->lastIndexOf(&DOT);
	if(index < 0){
		return new UnicodeString(name);
	}

	return name->substring(index + 1);
}

#ifdef __DEBUG__
AnalyzedType* TypeResolver::testFindClassType(const UnicodeString* packageName, const UnicodeString* name) const {
	return findClassType(packageName, name);
}

AnalyzedType* TypeResolver::testResolveType(CodeElement* element, ObjectType* type) const {
	return resolveClassType(element, type);
}
#endif

} /* namespace alinous */
