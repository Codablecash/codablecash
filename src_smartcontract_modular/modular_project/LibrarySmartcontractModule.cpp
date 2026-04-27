/*
 * LibrarySmartcontractModule.cpp
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */

#include "modular_project/LibrarySmartcontractModule.h"
#include "modular_project/ModularConfigException.h"

#include "smartcontract_instance/LibraryExectableModuleInstance.h"

#include "json/JsonHandler.h"

#include "json_object/AbstractJsonObject.h"
#include "json_object/JsonObject.h"
#include "json_object/JsonStringValue.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonArrayObject.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "base_io/File.h"

#include "bc/ExceptionThrower.h"


namespace codablecash {

LibrarySmartcontractModule::LibrarySmartcontractModule(const UnicodeString* projectRelativePath)
		: AbstractSmartcontractModule(projectRelativePath) {
	this->libraryName = nullptr;
	this->exportClasses = new ArrayList<UnicodeString>();
}

LibrarySmartcontractModule::~LibrarySmartcontractModule() {
	delete this->libraryName;

	this->exportClasses->deleteElements();
	delete this->exportClasses;
}

void LibrarySmartcontractModule::load(const File *modulePath) {
	File* configJason = modulePath->get(CONFIG_JSON); __STP(configJason);

	JsonHandler handler;
	handler.loadFile(configJason, 256);
	AbstractJsonObject* object = handler.parse(); __STP(object);

	const JsonObject* root = dynamic_cast<JsonObject*>(object);
	ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(root == nullptr, L"Config file must be json format.", __FILE__, __LINE__);

	// libraryName
	{
		const JsonValuePair* pair = root->get(LIBRARY_NAME);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The libraryName is essential.", __FILE__, __LINE__);

		JsonStringValue* strValue = dynamic_cast<JsonStringValue*>(pair->getValue());
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(strValue == nullptr, L"The libraryName must be String.", __FILE__, __LINE__);

		const UnicodeString* libname = strValue->getValue();
		setLibraryName(libname);
	}

	// exportClasses
	{
		const JsonValuePair* pair = root->get(EXPORT_CLASSES);
		if(pair != nullptr){
			AbstractJsonObject* obj = pair->getValue();
			JsonArrayObject* exportClasses = dynamic_cast<JsonArrayObject*>(obj);
			ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(exportClasses == nullptr, L"The exportClasses must be array object.", __FILE__, __LINE__);

			int maxLoop = exportClasses->size();
			for(int i = 0; i != maxLoop; ++i){
				AbstractJsonObject* element = exportClasses->get(i);
				JsonStringValue* strValue = dynamic_cast<JsonStringValue*>(element);

				ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(strValue == nullptr, L"The element of exportClasses must be a String object.", __FILE__, __LINE__);
				const UnicodeString* str = strValue->getValue();

				this->exportClasses->addElement(new UnicodeString(str));
			}
		}
	}

	analyzeJsonObject(root);
}

void LibrarySmartcontractModule::setLibraryName(const UnicodeString *str) noexcept {
	delete this->libraryName;
	this->libraryName = new UnicodeString(str);
}

AbstractExecutableModuleInstance* LibrarySmartcontractModule::toInstance() const {
	LibraryExectableModuleInstance* inst = new LibraryExectableModuleInstance(); __STP(inst);
	setupInstance(inst);

	inst->setLibraryName(this->libraryName);

	int maxLoop = this->exportClasses->size();
	for(int i = 0; i != maxLoop; ++i){
		const UnicodeString* clazz = this->exportClasses->get(i);
		inst->addExportClass(clazz);
	}

	return __STP_MV(inst);
}



} /* namespace codablecash */
