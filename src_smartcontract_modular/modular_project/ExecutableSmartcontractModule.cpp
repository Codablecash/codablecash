/*
 * ExecutableSmartcontractModule.cpp
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */

#include "modular_project/ExecutableSmartcontractModule.h"
#include "modular_project/ModularConfigException.h"

#include "smartcontract_instance/ExecutableModuleInstance.h"

#include "json/JsonHandler.h"

#include "json_object/AbstractJsonObject.h"
#include "json_object/JsonObject.h"

#include "base/StackRelease.h"

#include "base_io/File.h"

#include "bc/ExceptionThrower.h"


namespace codablecash {

ExecutableSmartcontractModule::ExecutableSmartcontractModule(const UnicodeString* projectRelativePath)
		: AbstractSmartcontractModule(projectRelativePath) {

}

ExecutableSmartcontractModule::~ExecutableSmartcontractModule() {

}

void ExecutableSmartcontractModule::load(const File *modulePath) {
	File* configJason = modulePath->get(CONFIG_JSON); __STP(configJason);

	JsonHandler handler;
	handler.loadFile(configJason, 256);
	AbstractJsonObject* object = handler.parse(); __STP(object);

	const JsonObject* root = dynamic_cast<JsonObject*>(object);
	ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(root == nullptr, L"Config file must be json format.", __FILE__, __LINE__);

	analyzeJsonObject(root);
}

AbstractExecutableModuleInstance* ExecutableSmartcontractModule::toInstance() const {
	ExecutableModuleInstance* inst = new ExecutableModuleInstance(); __STP(inst);
	setupInstance(inst);

	return __STP_MV(inst);
}

} /* namespace codablecash */
