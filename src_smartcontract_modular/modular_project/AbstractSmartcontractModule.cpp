/*
 * AbstractModule.cpp
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */

#include "modular_project/AbstractSmartcontractModule.h"
#include "modular_project/DependencyConfig.h"
#include "modular_project/ModularConfigException.h"
#include "modular_project/ModularInstanceConfig.h"

#include "base/UnicodeString.h"

#include "bc/SoftwareVersion.h"
#include "bc/ExceptionThrower.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonArrayObject.h"
#include "json_object/JsonStringValue.h"

#include "smartcontract_instance/AbstractExecutableModuleInstance.h"

namespace codablecash {

AbstractSmartcontractModule::AbstractSmartcontractModule(const UnicodeString* projectRelativePath) {
	this->projectRelativePath = new UnicodeString(projectRelativePath);

	this->sourceFolders = new ArrayList<UnicodeString>();

	this->instance = nullptr;

	this->version = nullptr;
	this->dependencies = nullptr;
}

AbstractSmartcontractModule::~AbstractSmartcontractModule() {
	delete this->projectRelativePath;

	this->sourceFolders->deleteElements();
	delete this->sourceFolders;

	delete this->instance;

	delete this->version;

	delete this->dependencies;
}

void AbstractSmartcontractModule::analyzeJsonObject(const JsonObject *object) {
	// sourceFolders
	{
		const JsonValuePair* pair = object->get(SOURCE_FOLDERS);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The sourceFilder is essential.", __FILE__, __LINE__);

		AbstractJsonObject* value = pair->getValue();
		JsonArrayObject* array = dynamic_cast<JsonArrayObject*>(value);

		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(array == nullptr, L"The sourceFilder must be array.", __FILE__, __LINE__);

		int maxLoop = array->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractJsonObject* element = array->get(i);
			JsonStringValue* value = dynamic_cast<JsonStringValue*>(element);

			ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(value == nullptr, L"The sourceFilder must be String.", __FILE__, __LINE__);

			const UnicodeString* str = value->getValue();
			this->sourceFolders->addElement(new UnicodeString(str));
		}
	}

	// version
	{
		const JsonValuePair* pair = object->get(VERSION);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The version is essential.", __FILE__, __LINE__);

		JsonStringValue* strValue = dynamic_cast<JsonStringValue*>(pair->getValue());

		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(strValue == nullptr, L"The version must be String.", __FILE__, __LINE__);

		const UnicodeString* str = strValue->getValue();
		this->version = SoftwareVersion::parseString(str);
	}

	// instance
	{
		const JsonValuePair* pair = object->get(INSTANCE);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The instance is essential.", __FILE__, __LINE__);

		AbstractJsonObject* obj = pair->getValue();
		JsonObject* inst = dynamic_cast<JsonObject*>(obj);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(inst == nullptr, L"The instance must be json object.", __FILE__, __LINE__);

		this->instance = new ModularInstanceConfig();
		this->instance->load(inst);
	}

	// dependencies
	{
		const JsonValuePair* pair = object->get(DEPENDENCIES);
		if(pair != nullptr){
			AbstractJsonObject* obj = pair->getValue();
			JsonArrayObject* dependencies = dynamic_cast<JsonArrayObject*>(obj);
			ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(dependencies == nullptr, L"The dependencies must be array object.", __FILE__, __LINE__);

			this->dependencies = new DependencyConfig();
			this->dependencies->load(dependencies);
		}
	}

}

void AbstractSmartcontractModule::setupInstance(AbstractExecutableModuleInstance *inst) const {
	{
		inst->setProjectRelativePath(this->projectRelativePath);
	}


	int maxLoop = this->sourceFolders->size();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* path = this->sourceFolders->get(i);
		inst->addSourceFolders(path);
	}

	inst->setSoftwareVersion(this->version);

	inst->setInstanceConfig(this->instance);
	inst->setDependencyConfig(this->dependencies);
}

} /* namespace codablecash */
