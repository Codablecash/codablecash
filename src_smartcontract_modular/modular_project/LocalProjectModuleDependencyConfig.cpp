/*
 * LocalProjectModuleDependencyConfig.cpp
 *
 *  Created on: Nov 14, 2025
 *      Author: iizuka
 */

#include "modular_project/LocalProjectModuleDependencyConfig.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonStringValue.h"

#include "bc_base/BinaryUtils.h"

namespace codablecash {

LocalProjectModuleDependencyConfig::LocalProjectModuleDependencyConfig(const LocalProjectModuleDependencyConfig &inst)
		: AbstractDependencyConfig(inst) {
	this->localloc = inst.localloc != nullptr ? new UnicodeString(inst.localloc) : nullptr;
}

LocalProjectModuleDependencyConfig::LocalProjectModuleDependencyConfig() : AbstractDependencyConfig(TYPE_LOCAL_MODULE) {
	this->localloc = nullptr;
}

LocalProjectModuleDependencyConfig::~LocalProjectModuleDependencyConfig() {
	delete this->localloc;
}

AbstractDependencyConfig* LocalProjectModuleDependencyConfig::copy() const {
	return new LocalProjectModuleDependencyConfig(*this);
}

LocalProjectModuleDependencyConfig* LocalProjectModuleDependencyConfig::fromJsonObject(const JsonObject *jsonObj) {
	LocalProjectModuleDependencyConfig* inst = new LocalProjectModuleDependencyConfig(); __STP(inst);

	fromJson(inst, jsonObj);

	{
		const JsonValuePair* p = jsonObj->get(LOCALLOC); // existence is proved
		AbstractJsonObject* jvalue = p->getValue();
		JsonStringValue* jstr = dynamic_cast<JsonStringValue*>(jvalue);

		const UnicodeString* lloc = jstr->getValue();
		inst->setLocalLoc(lloc);
	}

	return __STP_MV(inst);
}

void LocalProjectModuleDependencyConfig::setLocalLoc(const UnicodeString *lloc) noexcept {
	delete this->localloc;
	this->localloc = new UnicodeString(lloc);
}

int LocalProjectModuleDependencyConfig::binarySize() const {
	BinaryUtils::checkNotNull(this->localloc);

	int total = AbstractDependencyConfig::binarySize();
	total += BinaryUtils::stringSize(this->localloc);

	return total;
}

void LocalProjectModuleDependencyConfig::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->localloc);

	AbstractDependencyConfig::toBinary(out);
	BinaryUtils::putString(out, this->localloc);
}

void LocalProjectModuleDependencyConfig::fromBinary(ByteBuffer *in) {
	AbstractDependencyConfig::fromBinary(in);
	this->localloc = BinaryUtils::getString(in);
}

} /* namespace codablecash */
