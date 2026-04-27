/*
 * PackageSpace.cpp
 *
 *  Created on: 2019/06/01
 *      Author: iizuka
 */

#include "engine/sc_analyze/PackageSpace.h"
#include "engine/sc_analyze/AnalyzedClass.h"

#include "base/UnicodeString.h"

#include "lang/sc_declare/ClassDeclare.h"

#include "base/ArrayList.h"
#include "base/StackRelease.h"

namespace alinous {

PackageSpace::PackageSpace(const UnicodeString* name) {
	if(name == nullptr){
		UnicodeString tmp(L"");
		this->name = new UnicodeString(tmp);
	}
	else{
		this->name = new UnicodeString(*name);
	}

	this->classes = new HashMap<UnicodeString, AnalyzedClass>();
}

PackageSpace::~PackageSpace() {
	delete this->name;

	Iterator<UnicodeString>* it = this->classes->keySet()->iterator();
	while(it->hasNext()){
		const UnicodeString* n = it->next();
		AnalyzedClass* cls = this->classes->get(n);

		if(!cls->isReserved()){
			delete cls;
		}
	}
	delete it;

	delete this->classes;
}

void PackageSpace::addClassDeclare(ClassDeclare* clazz) noexcept {
#ifdef __DEBUG__
	AnalyzedClass* ac = this->classes->get(clazz->getName());
	assert(ac == nullptr);
#endif

	AnalyzedClass* aclazz = new AnalyzedClass(clazz);

	this->classes->put(clazz->getName(), aclazz);
}

void PackageSpace::addClassDeclare(AnalyzedClass* clazz) noexcept {
#ifdef __DEBUG__
	AnalyzedClass* ac = this->classes->get(clazz->getClassDeclare()->getName());
	assert(ac == nullptr);
#endif

	this->classes->put(clazz->getClassDeclare()->getName(), clazz);
}

AnalyzedClass* PackageSpace::getClass(const UnicodeString* name) noexcept {
	return this->classes->get(name);
}

void PackageSpace::analyzeClassInheritance(AnalyzeContext* actx) noexcept {
	Iterator<UnicodeString>* it = this->classes->keySet()->iterator();
	while(it->hasNext()){
		const UnicodeString* n = it->next();
		AnalyzedClass* cls = this->classes->get(n);

		doAnalyzeClassInheritance(cls);
		doAnalyzeDeliverClasses(cls);
	}

	delete it;
}

void PackageSpace::doAnalyzeClassInheritance(AnalyzedClass* cls) noexcept {
	ArrayList<AnalyzedClass> list;
	list.addElement(cls);

	AnalyzedClass* clazz = cls->getExtends();

	while(clazz != nullptr){
		list.addElement(clazz);
		clazz = clazz->getExtends();
	}

	int maxLoop = list.size();
	int idxCount = 0;
	for(int i = maxLoop - 1; i >= 0; --i){
		clazz = list.get(i);

		int idx = clazz->getInheritIndex();
		assert(idx < 0 || idx == idxCount);

		clazz->setInheritIndex(idxCount);
		idxCount++;
	}
}

void PackageSpace::doAnalyzeDeliverClasses(AnalyzedClass *cls) noexcept {
	// down analysis
	ArrayList<AnalyzedClass> list;

	{
		AnalyzedClass* clazz = cls;
		while(clazz != nullptr){
			const ArrayList<AnalyzedClass>* interfaces = clazz->getImplements();
			int	maxLoop = interfaces->size();

			list.addElement(clazz, 0);

			for(int i = 0; i != maxLoop; ++i){
				AnalyzedClass* ifclazz = interfaces->get(i);

				analyzeInterfaceInheritance(ifclazz, &list);
			}

			clazz = clazz->getExtends();
		}
	}


	// up analysis
	{
		int maxLoop = list.size();
		for(int i = 0; i != maxLoop; ++i){
			AnalyzedClass* clazz = list.get(i);

			addDeliverClasses(clazz, &list, i+1);
		}
	}
}

void PackageSpace::addDeliverClasses(AnalyzedClass *targetClazz, ArrayList<AnalyzedClass> *list, int pos) {
	int maxLoop = list->size();
	for(int i = pos; i < maxLoop; ++i){
		AnalyzedClass* clazz = list->get(i);

		if(!clazz->isInterface()){
			targetClazz->addDelivedImplementClass(clazz);
		}
	}
}

void PackageSpace::analyzeInterfaceInheritance(AnalyzedClass *ifclazz, ArrayList<AnalyzedClass> *list) {
	while(ifclazz != nullptr && ifclazz->isInterface()){
		list->addElement(ifclazz, 0);
		ifclazz = ifclazz->getExtends();
	}
}

void PackageSpace::buildVTables(AnalyzeContext* actx) noexcept {
	Iterator<UnicodeString>* it = this->classes->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* n = it->next();
		AnalyzedClass* cls = this->classes->get(n);

		cls->buildVtable(actx);
	}
}

HashMap<UnicodeString, AnalyzedClass>* PackageSpace::getMap() const noexcept {
	return this->classes;
}

bool PackageSpace::isEmpty() const noexcept {
	return this->classes->isEmpty();
}

} /* namespace alinous */
