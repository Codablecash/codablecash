/*
 * FinalizerConfig.cpp
 *
 *  Created on: 2023/12/09
 *      Author: iizuka
 */

#include "bc_network_instance/FinalizerConfig.h"

#include "bc_network/NodeIdentifierSource.h"

#include "base/StackRelease.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonStringValue.h"
#include "json_object/JsonValuePair.h"

#include "bc/ExceptionThrower.h"

#include "sim/SimnetConfigException.h"
namespace codablecash {

FinalizerConfig::FinalizerConfig(const FinalizerConfig &inst) {
	this->voterSource = inst.voterSource != nullptr ? dynamic_cast<NodeIdentifierSource*>(inst.voterSource->copyData()) : nullptr;
}

FinalizerConfig::FinalizerConfig() {
	this->voterSource = nullptr;
}

FinalizerConfig::~FinalizerConfig() {
	delete this->voterSource;
}

void FinalizerConfig::setVoterSource(const NodeIdentifierSource *source) noexcept {
	delete this->voterSource;
	this->voterSource = dynamic_cast<NodeIdentifierSource*>(source->copyData());
}

JsonObject* FinalizerConfig::toJsonObject() const {
	JsonObject* configObj = new JsonObject(); __STP(configObj);
	if(this->voterSource != nullptr){
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"voter"));
		pair->setValue(this->voterSource->toJsonObject());

		configObj->add(pair);
	}

	return __STP_MV(configObj);
}

FinalizerConfig* FinalizerConfig::fromJson(const JsonObject *configObj) {
	FinalizerConfig* config = new FinalizerConfig(); __STP(config);

	const JsonValuePair* pair = configObj->get(L"voter");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonObject* value = dynamic_cast<JsonObject*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"voter of the Finalizer Config must be object type.", __FILE__, __LINE__);

		NodeIdentifierSource* source = NodeIdentifierSource::fromJson(value); __STP(source);
		config->setVoterSource(source);
	}

	return __STP_MV(config);
}

} /* namespace codablecash */
