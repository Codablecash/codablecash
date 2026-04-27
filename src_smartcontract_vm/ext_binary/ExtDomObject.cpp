/*
 * ExtDomObject.cpp
 *
 *  Created on: 2020/06/25
 *      Author: iizuka
 */

#include "ext_binary/ExtDomObject.h"
#include "ext_binary/AbstractExtObject.h"

#include "instance/VmInstanceTypesConst.h"

#include "instance/instance_dom/DomVariableInstance.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"


namespace alinous {

ExtDomObject::ExtDomObject(const UnicodeString* name) : AbstractExtObject(name, VmInstanceTypesConst::INST_DOM) {
	this->properties = new HashMap<UnicodeString, AbstractExtObject>();
}

ExtDomObject::~ExtDomObject() {
	Iterator<UnicodeString>* it = this->properties->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		AbstractExtObject* exobj  = this->properties->get(key);

		delete exobj;
	}

	delete this->properties;
}

void ExtDomObject::put(const UnicodeString* name, AbstractExtObject* value) noexcept {
	value->setName(name);
	this->properties->put(name, value);
}

AbstractExtObject* ExtDomObject::get(const UnicodeString* name) const noexcept {
	return this->properties->get(name);
}

AbstractExtObject* ExtDomObject::get(const wchar_t* name) const noexcept {
	UnicodeString str(name);

	return get(&str);
}

AbstractExtObject* ExtDomObject::copy() const noexcept {
	ExtDomObject* newObj = new ExtDomObject(getName());

	Iterator<UnicodeString>* it = this->properties->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		AbstractExtObject* exobj  = this->properties->get(key);

		newObj->put(key, exobj->copy());
	}

	return newObj;
}

const UnicodeString* ExtDomObject::toString() const noexcept {
	delete this->str;
	this->str = new UnicodeString(L"{");

	int i = 0;
	Iterator<UnicodeString>* it = this->properties->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		AbstractExtObject* exobj  = this->properties->get(key);

		if(i > 0){
			this->str->append(L", ");
		}

		this->str->append(key);
		this->str->append(L": ");
		this->str->append(exobj->toString());

		i++;
	}

	this->str->append(L"}");

	return this->str;
}

AbstractVmInstance* ExtDomObject::toVmInstance(VirtualMachine *vm) {
	DomVariableInstance* vmInstance = new(vm) DomVariableInstance(vm); __STP(vmInstance);

	Iterator<UnicodeString>* it = this->properties->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const UnicodeString* key = it->next();
		AbstractExtObject* exobj  = this->properties->get(key);

		AbstractVmInstance* inst = exobj->toVmInstance(vm);
		IAbstractVmInstanceSubstance* substance = inst->getInstance();

		VmStringInstance* vmString = new(vm) VmStringInstance(vm, key); __STP(vmString);
		vmInstance->putProperty(vm, vmString, substance);
	}

	return __STP_MV(vmInstance);
	// FIXME toVmInstance
}

} /* namespace alinous */
