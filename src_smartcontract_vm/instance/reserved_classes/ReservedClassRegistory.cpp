/*
 * ReservedClassRegistory.cpp
 *
 *  Created on: 2020/03/16
 *      Author: iizuka
 */

#include "instance/reserved_classes/ReservedClassRegistory.h"

#include "engine/sc_analyze/AnalyzedClass.h"

#include "instance/reserved_classes_string/StringClassDeclare.h"

#include "instance/instance_exception_class/ExceptionClassDeclare.h"

#include "engine/sc/CompilationUnit.h"

#include "instance/instance_exception/ArrayOutOfBoundsExceptionClassDeclare.h"
#include "instance/instance_exception/NullPointerExceptionClassDeclare.h"
#include "instance/instance_exception/ZeroDivisionExceptionClassDeclare.h"
#include "instance/instance_exception/TypeCastExceptionClassDeclare.h"
#include "trx/transaction_exception/DatabaseExceptionClassDeclare.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "instance/reserved_classes/object/ObjectClassDeclare.h"

#include "instance/reserved_classes/list/ListClassDeclare.h"

#include "lang/sc_declare/PackageDeclare.h"
#include "lang/sc_declare/PackageNameDeclare.h"


#include "inter_modular_access/InterModuleAccessException.h"

#include "modular_interfaces/ModularProxyListnerClassDeclare.h"
using namespace codablecash;

namespace alinous {

ReservedClassRegistory::ReservedClassRegistory() {
	this->unitlist = new ArrayList<CompilationUnit>();

	AnalyzedClass* aclass = StringClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);

	aclass = ExceptionClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);

	aclass = ArrayOutOfBoundsExceptionClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);

	aclass = NullPointerExceptionClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);

	aclass = ZeroDivisionExceptionClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);

	aclass = TypeCastExceptionClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);

	aclass = DatabaseExceptionClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);

	aclass = InterModuleAccessException::createAnalyzedClass();
	addAnalyzedClass(aclass);

	// Object Class
	aclass = ObjectClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);

	// List Class (Generics)
	aclass = ListClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);

	// ModularProxyListnerClassDeclare
	aclass = ModularProxyListnerClassDeclare::createAnalyzedClass();
	addAnalyzedClass(aclass);
}

AnalyzedClass* ReservedClassRegistory::getAnalyzedClass(const UnicodeString* fqn) const noexcept {
	return this->map.get(fqn);
}

void ReservedClassRegistory::addAnalyzedClass(AnalyzedClass* aclass) noexcept {
	aclass->setReserved(true);
	this->list.addElement(aclass);

	const UnicodeString* name = aclass->getFullQualifiedName();
	this->map.put(name, aclass);
}

ReservedClassRegistory::~ReservedClassRegistory() {
	this->list.deleteElements();

	this->unitlist->deleteElements();
	delete this->unitlist;
}

const ArrayList<AnalyzedClass>* ReservedClassRegistory::getReservedClassesList() const noexcept {
	return &this->list;
}

CompilationUnit* ReservedClassRegistory::makeCompilantUnit(const UnicodeString *packageName) noexcept {
	CompilationUnit* unit = new CompilationUnit();

	this->unitlist->addElement(unit);

	if(packageName != nullptr){
		PackageDeclare* packageDec = new PackageDeclare();

		PackageNameDeclare* nameDec = new PackageNameDeclare();
		nameDec->setParent(packageDec);

		UnicodeString pattern(L"\\.");
		ArrayList<UnicodeString>* list = packageName->split(&pattern); __STP(list);
		list->setDeleteOnExit();

		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			UnicodeString* seg = list->get(i);

			nameDec->addSegment(new UnicodeString(seg));
		}

		packageDec->setName(nameDec);
		unit->setPackage(packageDec);
	}

	return unit;
}

void ReservedClassRegistory::initCompilantUnits(VirtualMachine *vm) {
	int maxLoop = this->unitlist->size();
	for(int i = 0; i != maxLoop; ++i){
		CompilationUnit* unit = this->unitlist->get(i);
		unit->init(vm);
	}
}

} /* namespace alinous */
