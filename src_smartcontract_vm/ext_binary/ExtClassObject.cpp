/*
 * ExtClassObject.cpp
 *
 *  Created on: 2019/12/18
 *      Author: iizuka
 */

#include "ext_binary/ExtClassObject.h"
#include "ext_binary/ExtPrimitiveObject.h"
#include "ext_binary/ExtArrayObject.h"
#include "ext_binary/ExtStringClass.h"
#include "ext_binary/ExtExceptionObject.h"
#include "ext_binary/ExtDomObject.h"
#include "ext_binary/ExtDomArrayObject.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "instance/VmInstanceTypesConst.h"
#include "instance/VmClassInstance.h"

#include "instance/instance_ref/ObjectReference.h"

#include "vm/VirtualMachine.h"
#include "vm/exceptions.h"

#include "engine/sc/SmartContract.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/TypeResolver.h"
#include "engine/sc_analyze/PackageSpace.h"

#include "bc/ExceptionThrower.h"

#include "instance/IAbstractVmInstanceSubstance.h"

#include "instance/instance_gc/GcManager.h"


using namespace codablecash;

namespace alinous {

ExtClassObject::ExtClassObject(const UnicodeString* name, const UnicodeString* fqn) : AbstractExtObject(name, VmInstanceTypesConst::INST_OBJ) {
	this->list = new ArrayList<AbstractExtObject>();
	this->map = new HashMap<UnicodeString, AbstractExtObject>();
	this->fqn = new UnicodeString(fqn);
}

ExtClassObject::~ExtClassObject() {
	this->list->deleteElements();

	delete this->list;
	delete this->map;

	delete this->fqn;
}

void ExtClassObject::add(AbstractExtObject* obj) noexcept {
	this->list->addElement(obj);
	this->map->put(obj == nullptr ? nullptr : obj->getName(), obj);
}

ExtPrimitiveObject* ExtClassObject::getExtPrimitiveObject(const UnicodeString* name) const noexcept {
	AbstractExtObject* obj = this->map->get(name);
	return dynamic_cast<ExtPrimitiveObject*>(obj);
}

ExtClassObject* ExtClassObject::getExtClassObject(const UnicodeString* name) const noexcept {
	AbstractExtObject* obj = this->map->get(name);

	if(obj == nullptr || obj->isNull()){
		return nullptr;
	}

	return dynamic_cast<ExtClassObject*>(obj);
}

ExtClassObject* ExtClassObject::getExtClassObject(const wchar_t *name) const noexcept {
	UnicodeString __name(name);
	return getExtClassObject(&__name);
}

ExtExceptionObject* ExtClassObject::getExtExceptionObject(const UnicodeString* name) const noexcept {
	AbstractExtObject* obj = this->map->get(name);

	if(obj == nullptr || obj->isNull()){
		return nullptr;
	}

	return dynamic_cast<ExtExceptionObject*>(obj);
}

ExtArrayObject* ExtClassObject::getExtArrayObject(const UnicodeString* name) const noexcept {
	AbstractExtObject* obj = this->map->get(name);
	return dynamic_cast<ExtArrayObject*>(obj);
}

ExtStringClass* alinous::ExtClassObject::getExtStringObject(const UnicodeString* name) const noexcept {
	AbstractExtObject* obj = this->map->get(name);

	if(obj->isNull()){
		return nullptr;
	}

	return dynamic_cast<ExtStringClass*>(obj);
}

ExtDomObject* ExtClassObject::getExtDomObject(const UnicodeString* name) const noexcept {
	AbstractExtObject* obj = this->map->get(name);

	return obj->isNull() ? nullptr : dynamic_cast<ExtDomObject*>(obj);
}

ExtDomArrayObject* ExtClassObject::getExtDomArrayObject(const UnicodeString* name) const noexcept {
	AbstractExtObject* obj = this->map->get(name);

	return (obj == nullptr || obj->isNull()) ? nullptr : dynamic_cast<ExtDomArrayObject*>(obj);
}

AbstractExtObject* ExtClassObject::copy() const noexcept {
	ExtClassObject* newObj = new ExtClassObject(this->name, this->fqn);

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExtObject* obj = this->list->get(i);

		newObj->add(obj == nullptr ? nullptr : obj->copy());
	}

	return newObj;
}

const UnicodeString* ExtClassObject::toString() const noexcept {
	delete this->str;
	this->str = new UnicodeString(L"class {");

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExtObject* obj = this->list->get(i);

		if(i > 0){
			this->str->append(L", ");
		}

		this->str->append(obj->toString());
	}

	this->str->append(L"}");

	return this->str;
}

AbstractVmInstance* ExtClassObject::toVmInstance(VirtualMachine *vm) {
	SmartContract* contract = vm->getSmartContract();
	AnalyzeContext* actx = contract->getAnalyzeContext();
	GcManager* gc = vm->getGc();

	UnicodeString* packageName = TypeResolver::getPackageName(this->fqn); __STP(packageName);
	PackageSpace* space = actx->getPackegeSpace(packageName);
	ExceptionThrower<VmClassNotFoundException>::throwExceptionIfCondition(space == nullptr, L"The package was not found.", __FILE__, __LINE__);

	UnicodeString* className = TypeResolver::getClassName(this->fqn); __STP(className);
	AnalyzedClass* aclass = space->getClass(className);
	ExceptionThrower<VmClassNotFoundException>::throwExceptionIfCondition(aclass == nullptr, L"The class was not found.", __FILE__, __LINE__);

	VmClassInstance* inst = new(vm) VmClassInstance(aclass, vm); __STP(inst);

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractExtObject* obj = this->list->get(i);

		AbstractVmInstance* memberValue = obj->toVmInstance(vm);
		IAbstractVmInstanceSubstance* substance = memberValue->getInstance();

		AbstractReference* ref = substance->wrap(inst, vm);

		inst->addMember(gc, ref);
	}

	return __STP_MV(inst);
}

} /* namespace alinous */
