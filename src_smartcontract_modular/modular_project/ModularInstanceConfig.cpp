/*
 * ModularInstance.cpp
 *
 *  Created on: Oct 8, 2025
 *      Author: iizuka
 */

#include "modular_project/ModularInstanceConfig.h"
#include "modular_project/ModularConfigException.h"

#include "json_object/AbstractJsonValue.h"
#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonArrayObject.h"
#include "json_object/JsonStringValue.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "bc/ExceptionThrower.h"

#include "bc_base/BinaryUtils.h"

#include "base_io/ByteBuffer.h"


namespace codablecash {

ModularInstanceConfig::ModularInstanceConfig() {
	this->mainPackage = nullptr;
	this->mainClass = nullptr;
	this->initializerMethod = nullptr;

	this->initializerMethodArguments = new ArrayList<AbstractJsonValue>();
	this->libExport = new ArrayList<UnicodeString>();
	this->directAccess = new ArrayList<UnicodeString>();

}

ModularInstanceConfig::~ModularInstanceConfig() {
	delete this->mainPackage;
	delete this->mainClass;
	delete this->initializerMethod;

	this->initializerMethodArguments->deleteElements();
	delete this->initializerMethodArguments;

	this->libExport->deleteElements();
	delete this->libExport;

	this->directAccess->deleteElements();
	delete this->directAccess;
}

void ModularInstanceConfig::load(const JsonObject *instance) {
	// mainPackage mainClass
	loadMainPackageAndClass(instance);

	// initializer
	{
		const JsonValuePair* pair = instance->get(INITIALIZER);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The initializer is essential.", __FILE__, __LINE__);

		AbstractJsonObject* object = pair->getValue();
		JsonObject* initializer = dynamic_cast<JsonObject*>(object);

		if(initializer != nullptr){
			loadInitializer(initializer);
		}
	}

	// libExport
	{
		const JsonValuePair* pair = instance->get(LIB_EXPORT);
		if(pair != nullptr){
			AbstractJsonObject* object = pair->getValue();
			JsonArrayObject* libExport = dynamic_cast<JsonArrayObject*>(object);

			if(libExport != nullptr){
				loadLibExport(libExport);
			}
		}
	}

	// directAccess
	{
		const JsonValuePair* pair = instance->get(DIRECT_ACCESS);
		if(pair != nullptr){
			AbstractJsonObject* object = pair->getValue();
			JsonArrayObject* directAccess = dynamic_cast<JsonArrayObject*>(object);

			if(directAccess != nullptr){
				loadDirectAccess(directAccess);
			}
		}
	}
}

void ModularInstanceConfig::loadMainPackageAndClass(const JsonObject *instance) {
	// mainPackage
	{
		const JsonValuePair* pair = instance->get(MAIN_PACKAGE);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The mainPackage is essential.", __FILE__, __LINE__);

		AbstractJsonObject* object = pair->getValue();
		JsonStringValue* jvalue = dynamic_cast<JsonStringValue*>(object);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(jvalue == nullptr, L"The mainPackage must be string value.", __FILE__, __LINE__);

		const UnicodeString* value = jvalue->getValue();
		setMainPackage(value);
	}

	// mainClass
	{
		const JsonValuePair* pair = instance->get(MAIN_CLASS);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The mainClass is essential.", __FILE__, __LINE__);

		AbstractJsonObject* object = pair->getValue();
		JsonStringValue* jvalue = dynamic_cast<JsonStringValue*>(object);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(jvalue == nullptr, L"The mainPackage must be string value.", __FILE__, __LINE__);

		const UnicodeString* value = jvalue->getValue();
		setMainClass(value);
	}
}

void ModularInstanceConfig::loadInitializer(const JsonObject *initializer) {
	// method
	{
		const JsonValuePair* pair = initializer->get(METHOD);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The method is essential.", __FILE__, __LINE__);

		AbstractJsonObject* object = pair->getValue();
		JsonStringValue* jvalue = dynamic_cast<JsonStringValue*>(object);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(jvalue == nullptr, L"The method must be string value.", __FILE__, __LINE__);

		const UnicodeString* value = jvalue->getValue();
		setInitializerMethod(value);
	}

	// args
	{
		const JsonValuePair* pair = initializer->get(ARGS);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The args is essential.", __FILE__, __LINE__);

		AbstractJsonObject* object = pair->getValue();
		JsonArrayObject* args = dynamic_cast<JsonArrayObject*>(object);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(args == nullptr, L"The args must be array value.", __FILE__, __LINE__);

		int maxLoop = args->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractJsonObject* object = args->get(i);
			AbstractJsonValue* value = dynamic_cast<AbstractJsonValue*>(object);

			ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(value == nullptr, L"The member of args must be value.", __FILE__, __LINE__);

			this->initializerMethodArguments->addElement(dynamic_cast<AbstractJsonValue*>(value->copy()));
		}
	}
}

void ModularInstanceConfig::loadLibExport(const JsonArrayObject *libExport) {
	int maxLoop = libExport->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractJsonObject* jobj = libExport->get(i);
		JsonStringValue* stringValue = dynamic_cast<JsonStringValue*>(jobj);

		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(stringValue == nullptr, L"The libExport must be string value.", __FILE__, __LINE__);

		const UnicodeString* __interface = stringValue->getValue();
		this->libExport->addElement(new UnicodeString(__interface));
	}
}

void ModularInstanceConfig::loadDirectAccess(const JsonArrayObject *directAccess) {
	int maxLoop = directAccess->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractJsonObject* jobj = directAccess->get(i);
		JsonStringValue* stringValue = dynamic_cast<JsonStringValue*>(jobj);

		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(stringValue == nullptr, L"The directAccess must be string value.", __FILE__, __LINE__);

		const UnicodeString* __interface = stringValue->getValue();
		this->directAccess->addElement(new UnicodeString(__interface));
	}
}

void ModularInstanceConfig::setMainPackage(const UnicodeString *value) noexcept {
	delete this->mainPackage;
	this->mainPackage = new UnicodeString(value);
}

void ModularInstanceConfig::setMainClass(const UnicodeString *value) noexcept {
	delete this->mainClass;
	this->mainClass = new UnicodeString(value);
}

void ModularInstanceConfig::setInitializerMethod(const UnicodeString *value) noexcept {
	delete this->initializerMethod;
	this->initializerMethod = new UnicodeString(value);
}

int ModularInstanceConfig::binarySize() const {
	BinaryUtils::checkNotNull(this->mainPackage);
	BinaryUtils::checkNotNull(this->mainClass);
	BinaryUtils::checkNotNull(this->initializerMethod);

	int total = BinaryUtils::stringSize(this->mainPackage);
	total += BinaryUtils::stringSize(this->mainClass);
	total += BinaryUtils::stringSize(this->initializerMethod);

	int maxLoop = this->initializerMethodArguments->size();
	total += sizeof(uint16_t);
	for(int i = 0; i != maxLoop; ++i){
		AbstractJsonValue* value = this->initializerMethodArguments->get(i);

		total += value->binarySize();
	}

	maxLoop = this->libExport->size();
	total += sizeof(uint16_t);
	for(int i = 0; i != maxLoop; ++i){
		const UnicodeString* element = this->libExport->get(i);
		total += BinaryUtils::stringSize(element);
	}

	maxLoop = this->directAccess->size();
	total += sizeof(uint16_t);
	for(int i = 0; i != maxLoop; ++i){
		const UnicodeString* element = this->directAccess->get(i);
		total += BinaryUtils::stringSize(element);
	}

	return total;
}

void ModularInstanceConfig::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->mainPackage);
	BinaryUtils::checkNotNull(this->mainClass);
	BinaryUtils::checkNotNull(this->initializerMethod);

	BinaryUtils::putString(out, this->mainPackage);
	BinaryUtils::putString(out, this->mainClass);
	BinaryUtils::putString(out, this->initializerMethod);

	int maxLoop = this->initializerMethodArguments->size();
	out->putShort(maxLoop);
	for(int i = 0; i != maxLoop; ++i){
		AbstractJsonValue* value = this->initializerMethodArguments->get(i);

		value->toBinary(out);
	}

	maxLoop = this->libExport->size();
	out->putShort(maxLoop);
	for(int i = 0; i != maxLoop; ++i){
		const UnicodeString* element = this->libExport->get(i);
		BinaryUtils::putString(out, element);
	}

	maxLoop = this->directAccess->size();
	out->putShort(maxLoop);
	for(int i = 0; i != maxLoop; ++i){
		const UnicodeString* element = this->directAccess->get(i);
		BinaryUtils::putString(out, element);
	}
}

ModularInstanceConfig* ModularInstanceConfig::createFromBinary(ByteBuffer *in) {
	ModularInstanceConfig* inst = new ModularInstanceConfig(); __STP(inst);

	inst->fromBinary(in);

	return __STP_MV(inst);
}

void ModularInstanceConfig::fromBinary(ByteBuffer *in) {
	this->mainPackage = BinaryUtils::getString(in);
	this->mainClass = BinaryUtils::getString(in);
	this->initializerMethod = BinaryUtils::getString(in);

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		AbstractJsonObject* obj = AbstractJsonObject::createFromBinary(in); __STP(obj);
		AbstractJsonValue* value = dynamic_cast<AbstractJsonValue*>(obj);
		BinaryUtils::checkNotNull(value);
		__STP_MV(obj);

		this->initializerMethodArguments->addElement(value);
	}

	maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* element = BinaryUtils::getString(in);
		this->libExport->addElement(element);
	}

	maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* element = BinaryUtils::getString(in);
		this->directAccess->addElement(element);
	}
}

} /* namespace codablecash */
