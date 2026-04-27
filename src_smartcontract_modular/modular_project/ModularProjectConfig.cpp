/*
 * ModularProjectConfig.cpp
 *
 *  Created on: Oct 4, 2025
 *      Author: iizuka
 */

#include "modular_project/ModularProjectConfig.h"
#include "modular_project/ModularConfigException.h"

#include "json/JsonHandler.h"

#include "json_object/AbstractJsonObject.h"
#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonStringValue.h"
#include "json_object/JsonArrayObject.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "bc/ExceptionThrower.h"
#include "bc/SoftwareVersion.h"

#include "bc_base/BinaryUtils.h"

#include "base_io/ByteBuffer.h"

#include <cstdint>

namespace codablecash {

ModularProjectConfig::ModularProjectConfig() {
	this->projectName = nullptr;
	this->version = nullptr;
	this->executable = nullptr;

	this->libralies = new ArrayList<UnicodeString>();

	this->author = nullptr;
	this->license = nullptr;
}

ModularProjectConfig::~ModularProjectConfig() {
	delete this->projectName;
	delete this->version;
	delete this->executable;

	this->libralies->deleteElements();
	delete this->libralies;

	delete this->author;
	delete this->license;
}

void ModularProjectConfig::load(const File *file) {
	JsonHandler handler;
	handler.loadFile(file, 256);
	AbstractJsonObject* object = handler.parse(); __STP(object);

	const JsonObject* root = dynamic_cast<JsonObject*>(object);
	ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(root == nullptr, L"Config file must be json format.", __FILE__, __LINE__);

	// project
	{
		const JsonValuePair* pair = root->get(PROJECT);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The project is essential.", __FILE__, __LINE__);

		AbstractJsonObject* obj = pair->getValue();
		JsonStringValue* stringValue = dynamic_cast<JsonStringValue*>(obj);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(stringValue == nullptr, L"The project must be string value.", __FILE__, __LINE__);

		setProjectName(stringValue->getValue());
	}

	// version
	{
		const JsonValuePair* pair = root->get(VERSION);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The version is essential.", __FILE__, __LINE__);

		JsonStringValue* strValue = dynamic_cast<JsonStringValue*>(pair->getValue());

		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(strValue == nullptr, L"The version must be String.", __FILE__, __LINE__);

		const UnicodeString* str = strValue->getValue();
		this->version = SoftwareVersion::parseString(str);
	}

	// executable
	{
		const JsonValuePair* pair = root->get(EXECUTABLE);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The executable is essential.", __FILE__, __LINE__);

		AbstractJsonObject* obj = pair->getValue();
		JsonStringValue* stringValue = dynamic_cast<JsonStringValue*>(obj);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(stringValue == nullptr, L"The executable must be string value.", __FILE__, __LINE__);

		setExecutable(stringValue->getValue());
	}

	// author
	{
		const JsonValuePair* pair = root->get(AUTHOR);
		if(pair != nullptr){
			AbstractJsonObject* obj = pair->getValue();
			JsonStringValue* stringValue = dynamic_cast<JsonStringValue*>(obj);
			ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(stringValue == nullptr, L"The author must be string value.", __FILE__, __LINE__);

			setAuthor(stringValue->getValue());
		}
	}

	// license
	{
		const JsonValuePair* pair = root->get(LICENSE);
		if(pair != nullptr){
			AbstractJsonObject* obj = pair->getValue();
			JsonStringValue* stringValue = dynamic_cast<JsonStringValue*>(obj);
			ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(stringValue == nullptr, L"The license must be string value.", __FILE__, __LINE__);

			setLicense(stringValue->getValue());
		}
	}

	// libraries
	{
		const JsonValuePair* pair = root->get(LIBRALIES);
		if(pair != nullptr){
			AbstractJsonObject* obj = pair->getValue();
			JsonArrayObject* arrayObj = dynamic_cast<JsonArrayObject*>(obj);
			ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(arrayObj == nullptr, L"The libraries must be array value.", __FILE__, __LINE__);

			int maxLoop = arrayObj->size();
			for(int i = 0; i != maxLoop; ++i){
				AbstractJsonObject* arobj = arrayObj->get(i);
				JsonStringValue* p = dynamic_cast<JsonStringValue*>(arobj);
				ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(p == nullptr, L"The json file's format is broken.", __FILE__, __LINE__);

				const UnicodeString* libname = p->getValue();
				addLibrary(libname);
			}
		}
	}
}

void ModularProjectConfig::setProjectName(const UnicodeString *name) noexcept {
	delete this->projectName;
	this->projectName = new UnicodeString(name);
}

void ModularProjectConfig::setVersion(const SoftwareVersion *ver) noexcept {
	delete this->version;
	this->version = new SoftwareVersion(*ver);
}

void ModularProjectConfig::setExecutable(const UnicodeString *exec) noexcept {
	delete this->executable;
	this->executable = new UnicodeString(exec);
}

void ModularProjectConfig::setAuthor(const UnicodeString *author) noexcept {
	delete this->author;
	this->author = new UnicodeString(author);
}

void ModularProjectConfig::setLicense(const UnicodeString *license) noexcept {
	delete this->license;
	this->license = new UnicodeString(license);
}

void ModularProjectConfig::addLibrary(const UnicodeString *libname) noexcept {
	UnicodeString* str = new UnicodeString(libname);
	this->libralies->addElement(str);
}

int ModularProjectConfig::binarySize() const {
	BinaryUtils::checkNotNull(this->projectName);
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->executable);

	int total = BinaryUtils::stringSize(this->projectName);
	total += this->version->binarySize();
	total += BinaryUtils::stringSize(this->executable);

	total += sizeof(uint8_t);
	if(this->author != nullptr){
		total += BinaryUtils::stringSize(this->author);
	}

	total += sizeof(uint8_t);
	if(this->license != nullptr){
		total += BinaryUtils::stringSize(this->license);
	}

	int maxLoop = this->libralies->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* lib = this->libralies->get(i);
		total += BinaryUtils::stringSize(lib);
	}

	return total;
}

void ModularProjectConfig::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->projectName);
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->executable);

	BinaryUtils::putString(out, this->projectName);
	this->version->toBinary(out);
	BinaryUtils::putString(out, this->executable);

	uint8_t bl = (this->author != nullptr) ? 1 : 0;
	out->put(bl);
	if(bl > 0){
		BinaryUtils::putString(out, this->author);
	}

	bl = (this->license != nullptr) ? 1 : 0;
	out->put(bl);
	if(bl > 0){
		BinaryUtils::putString(out, this->license);
	}

	int maxLoop = this->libralies->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* lib = this->libralies->get(i);
		BinaryUtils::putString(out, lib);
	}
}

ModularProjectConfig* ModularProjectConfig::createFromBinary(ByteBuffer *in) {
	ModularProjectConfig* inst = new ModularProjectConfig(); __STP(inst);

	{
		UnicodeString* str = BinaryUtils::getString(in); __STP(str);
		inst->setProjectName(str);
	}

	{
		SoftwareVersion* ver = SoftwareVersion::createFromBinary(in); __STP(ver);
		inst->setVersion(ver);
	}

	{
		UnicodeString* str = BinaryUtils::getString(in); __STP(str);
		inst->setExecutable(str);
	}

	uint8_t bl = in->get();
	if(bl > 0){
		UnicodeString* str = BinaryUtils::getString(in); __STP(str);
		inst->setAuthor(str);
	}

	bl = in->get();
	if(bl > 0){
		UnicodeString* str = BinaryUtils::getString(in); __STP(str);
		inst->setLicense(str);
	}

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		UnicodeString* str = BinaryUtils::getString(in); __STP(str);
		inst->addLibrary(str);
	}

	return __STP_MV(inst);
}

ModularProjectConfig* ModularProjectConfig::copy() const {
	int cap = binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

	toBinary(buff);
	buff->position(0);

	ModularProjectConfig* config = createFromBinary(buff);
	return config;
}

} /* namespace codablecash */
