/*
 * ZoneInstanceConfig.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: iizuka
 */

#include "sim/ZoneInstanceConfig.h"
#include "sim/SimnetConfigException.h"

#include "bc_block_generator/MiningConfig.h"

#include "bc_network_instance/FinalizerConfig.h"
#include "bc_network_instance/GenesisBalanceConfig.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "bc_p2p_info/P2pNodeRecord.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonStringValue.h"
#include "json_object/JsonNumericValue.h"

#include "bc/ExceptionThrower.h"


namespace codablecash {

ZoneInstanceConfig::ZoneInstanceConfig(const ZoneInstanceConfig &inst) {
	this->genConfig = inst.genConfig != nullptr ? new GenesisBalanceConfig(*inst.genConfig) : nullptr;
	this->minerConfig = inst.minerConfig != nullptr ? new MiningConfig(*inst.minerConfig) : nullptr;
	this->fconfig = inst.fconfig != nullptr ? new FinalizerConfig(*inst.fconfig) : nullptr;
	this->name = inst.name != nullptr ? new UnicodeString(inst.name) : nullptr;
	this->protocol = inst.protocol;
	this->port = inst.port;
}

ZoneInstanceConfig::ZoneInstanceConfig() {
	this->genConfig = nullptr;
	this->minerConfig = nullptr;
	this->fconfig = nullptr;
	this->name = nullptr;
	this->protocol = P2pNodeRecord::TCP_IP_V6;
	this->port = 0;
}

ZoneInstanceConfig::~ZoneInstanceConfig() {
	delete this->genConfig;
	delete this->minerConfig;
	delete this->fconfig;
	delete this->name;
}

void ZoneInstanceConfig::setGenesisConfig(const GenesisBalanceConfig *genConfig) {
	delete this->genConfig;
	this->genConfig = new GenesisBalanceConfig(*genConfig);
}

void ZoneInstanceConfig::setMiningConfig(const MiningConfig *minerConfig) {
	delete this->minerConfig;
	this->minerConfig = new MiningConfig(*minerConfig);
}

void ZoneInstanceConfig::setFinalizerConfig(const FinalizerConfig *fconfig) {
	delete this->fconfig;
	this->fconfig = new FinalizerConfig(*fconfig);
}

void ZoneInstanceConfig::setName(const UnicodeString *name) {
	delete this->name;
	this->name = new UnicodeString(*name);
}

void ZoneInstanceConfig::setProtocol(int protocol) {
	this->protocol = protocol;
}

void ZoneInstanceConfig::setPort(int port) {
	this->port = port;
}

JsonObject* ZoneInstanceConfig::toJsonObject() const {
	JsonObject* configObj = new JsonObject(); __STP(configObj);

	// name
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"name"));
		pair->setValue(new JsonStringValue(this->name));
		configObj->add(pair);
	}
	// protocol
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"protocol"));

		JsonStringValue* v = this->protocol == P2pNodeRecord::TCP_IP_V4 ? new JsonStringValue(L"ipv4") : new JsonStringValue(L"ipv6");
		pair->setValue(v);
		configObj->add(pair);
	}
	// port
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"port"));
		pair->setValue(new JsonNumericValue(this->port));
		configObj->add(pair);
	}

	if(this->genConfig != nullptr){
		JsonValuePair* pair = new JsonValuePair(); __STP(pair);
		pair->setKey(new JsonStringValue(L"genesis"));
		pair->setValue(this->genConfig->toJsonObject());

		configObj->add(__STP_MV(pair));
	}
	if(this->minerConfig != nullptr){
		JsonValuePair* pair = new JsonValuePair(); __STP(pair);
		pair->setKey(new JsonStringValue(L"mining"));
		pair->setValue(this->minerConfig->toJsonObject());

		configObj->add(__STP_MV(pair));
	}
	if(this->fconfig != nullptr){
		JsonValuePair* pair = new JsonValuePair(); __STP(pair);
		pair->setKey(new JsonStringValue(L"staking"));
		pair->setValue(this->fconfig->toJsonObject());

		configObj->add(__STP_MV(pair));
	}

	return __STP_MV(configObj);
}

ZoneInstanceConfig* ZoneInstanceConfig::fromJson(const JsonObject *jobj) {
	ZoneInstanceConfig* config = new ZoneInstanceConfig(); __STP(config);

	const JsonValuePair* pair = jobj->get(L"name");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonStringValue* value = dynamic_cast<JsonStringValue*>(av);

		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"name of the Zone Instance must be String type.", __FILE__, __LINE__);
		config->setName(value->getValue());
	}
	pair = jobj->get(L"protocol");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonStringValue* value = dynamic_cast<JsonStringValue*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"protocol of the Zone Instance must be String type.", __FILE__, __LINE__);

		const UnicodeString* str = value->getValue();
		if(str->equals(L"ipv4")){
			config->setProtocol(P2pNodeRecord::TCP_IP_V4);
		}else{
			config->setProtocol(P2pNodeRecord::TCP_IP_V6);
		}
	}
	pair = jobj->get(L"port");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonNumericValue* value = dynamic_cast<JsonNumericValue*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"port of the Zone Instance must be Numeric type.", __FILE__, __LINE__);

		config->setPort(value->getValue());
	}

	pair = jobj->get(L"genesis");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonObject* value = dynamic_cast<JsonObject*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"genesis of the Zone Instance must be object type.", __FILE__, __LINE__);

		GenesisBalanceConfig* cfg = GenesisBalanceConfig::fromJson(value); __STP(cfg);
		config->setGenesisConfig(cfg);
	}
	pair = jobj->get(L"mining");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonObject* value = dynamic_cast<JsonObject*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"mining of the Zone Instance must be object type.", __FILE__, __LINE__);

		MiningConfig*cfg = MiningConfig::fromJson(value); __STP(cfg);
		config->setMiningConfig(cfg);
	}
	pair = jobj->get(L"staking");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonObject* value = dynamic_cast<JsonObject*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"staking of the Zone Instance must be object type.", __FILE__, __LINE__);

		FinalizerConfig* cfg = FinalizerConfig::fromJson(value); __STP(cfg);
		config->setFinalizerConfig(cfg);
	}

	return __STP_MV(config);
}

} /* namespace codablecash */
