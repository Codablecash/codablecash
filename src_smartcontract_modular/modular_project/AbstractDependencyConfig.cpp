/*
 * AbstractDependencyConfig.cpp
 *
 *  Created on: Oct 8, 2025
 *      Author: iizuka
 */

#include "modular_project/AbstractDependencyConfig.h"
#include "modular_project/ModularSmartcontractVersion.h"
#include "modular_project/ModularConfigException.h"
#include "modular_project/LocalProjectModuleDependencyConfig.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonStringValue.h"

#include "bc/ExceptionThrower.h"

#include "base_io/ByteBuffer.h"

#include "bc_base/BinaryUtils.h"

namespace codablecash {


AbstractDependencyConfig::AbstractDependencyConfig(const AbstractDependencyConfig &inst) {
	this->kind = inst.kind;
	this->moduleName = new UnicodeString(inst.moduleName);
	this->version = new ModularSmartcontractVersion(*inst.version);
}

AbstractDependencyConfig::AbstractDependencyConfig(uint8_t kind) {
	this->kind = kind;
	this->moduleName = nullptr;
	this->version = nullptr;
}

AbstractDependencyConfig::~AbstractDependencyConfig() {
	delete this->moduleName;
	delete this->version;
}

void AbstractDependencyConfig::fromJson(AbstractDependencyConfig *inst, const JsonObject *jsonObj) {
	{
		const JsonValuePair* pair = jsonObj->get(NAME);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The name is essential for dependency config.", __FILE__, __LINE__);

		AbstractJsonObject* obj = pair->getValue();
		JsonStringValue* str = dynamic_cast<JsonStringValue*>(obj);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The name of dependency config must be string value.", __FILE__, __LINE__);

		const UnicodeString* strcValue = str->getValue();
		inst->setModuleName(strcValue);
	}

	{
		const JsonValuePair* pair = jsonObj->get(VERSION);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The version is essential for dependency config.", __FILE__, __LINE__);

		AbstractJsonObject* obj = pair->getValue();
		JsonStringValue* str = dynamic_cast<JsonStringValue*>(obj);
		ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(pair == nullptr, L"The name of dependency config must be string value.", __FILE__, __LINE__);

		const UnicodeString* strValue = str->getValue();
		ModularSmartcontractVersion* ver = ModularSmartcontractVersion::parseString(strValue); __STP(ver);
		inst->setVersion(ver);
	}
}

void AbstractDependencyConfig::setModuleName(const UnicodeString *name) noexcept {
	delete this->moduleName;
	this->moduleName = new UnicodeString(name);
}

void AbstractDependencyConfig::setVersion(const ModularSmartcontractVersion *v) noexcept {
	delete this->version;
	this->version = new ModularSmartcontractVersion(*v);
}

int AbstractDependencyConfig::binarySize() const {
	BinaryUtils::checkNotNull(this->moduleName);
	BinaryUtils::checkNotNull(this->version);

	int total = sizeof(uint8_t);
	total += BinaryUtils::stringSize(this->moduleName);
	total += this->version->binarySize();

	return total;
}

void AbstractDependencyConfig::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->moduleName);
	BinaryUtils::checkNotNull(this->version);

	out->put(this->kind);
	BinaryUtils::putString(out, this->moduleName);
	this->version->toBinary(out);
}

void AbstractDependencyConfig::fromBinary(ByteBuffer *in) {
	this->moduleName = BinaryUtils::getString(in);
	this->version = ModularSmartcontractVersion::createFromBinary(in);
}

AbstractDependencyConfig* AbstractDependencyConfig::createFromBinary(ByteBuffer *in) {
	AbstractDependencyConfig* ret = nullptr;

	uint8_t kind = in->get();
	ret = new LocalProjectModuleDependencyConfig();

	ret->fromBinary(in);
	return ret;
}

} /* namespace codablecash */
