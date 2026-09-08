/*
 * MiningConfig.cpp
 *
 *  Created on: 2023/03/29
 *      Author: iizuka
 */

#include "bc_block_generator/MiningConfig.h"

#include "bc_base/AddressDescriptor.h"

#include "pow/PoWCalculator.h"

#include "pow_pool/NetworkPoWPool.h"

#include "ipconnect/IServerSocket.h"

#include "bc_p2p_info/P2pNodeRecord.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonStringValue.h"
#include "json_object/JsonNumericValue.h"

#include "sim/SimnetConfigException.h"

#include "bc/ExceptionThrower.h"
namespace codablecash {

MiningConfig::MiningConfig(const MiningConfig &inst) {
	this->addressDesc = inst.addressDesc != nullptr ? dynamic_cast<AddressDescriptor*>(inst.addressDesc->copyData()) : nullptr;
	this->calcType = inst.calcType;

	this->protocol = inst.protocol;
	this->srvhost = inst.srvhost != nullptr ? new UnicodeString(inst.srvhost) : nullptr;
	this->port = inst.port;
}

MiningConfig::MiningConfig() {
	this->addressDesc = nullptr;
	this->calcType = MiningConfig::TYPE_DEBUG;

	this->protocol = P2pNodeRecord::TCP_IP_V6;
	this->srvhost = nullptr;
	this->port = 18080;
}

MiningConfig::~MiningConfig() {
	delete this->addressDesc;
	delete this->srvhost;
}

void MiningConfig::setAddressDescriptor(const AddressDescriptor *addressDesc) noexcept {
	delete this->addressDesc;
	this->addressDesc = dynamic_cast<AddressDescriptor*>(addressDesc->copyData());
}

AbstractPoWCalculator* MiningConfig::getPoWCalculator(PoWManager* powManager) const noexcept {
	AbstractPoWCalculator* inst = nullptr;
	if(this->calcType == TYPE_POOL){
		inst = new NetworkPoWPool(powManager);
		initNetworkPool(dynamic_cast<NetworkPoWPool*>(inst));
	}else{
		inst = new PoWCalculator();
	}

	return inst;
}

void MiningConfig::setCalclatorType(int type) noexcept {
	this->calcType = type;
}

void MiningConfig::setProtocol(int type) {
	this->protocol = type;
}

void MiningConfig::setServerHost(const UnicodeString *host) {
	this->srvhost = new UnicodeString(host);
}

void MiningConfig::setPort(int port) {
	this->port = port;
}

void MiningConfig::initNetworkPool(NetworkPoWPool *pool) const {
	pool->setProtocol(this->protocol);
	pool->setServerHost(this->srvhost);
	pool->setPort(this->port);

}

JsonObject* MiningConfig::toJsonObject() const {
	JsonObject* configObj = new JsonObject(); __STP(configObj);

	// reward address
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"address"));

		const char* cstr = this->addressDesc->toCString();
		StackArrayRelease<const char> __cstr(cstr);

		UnicodeString str(cstr);
		pair->setValue(new JsonStringValue(&str));

		configObj->add(pair);
	}
	// calc Type
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"type"));

		UnicodeString* typeStr = this->calcType == TYPE_POOL ? new UnicodeString(L"pool") : new UnicodeString("debug");
		__STP(typeStr);

		pair->setValue(new JsonStringValue(typeStr));

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
	// host
	if(this->srvhost != nullptr){
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"host"));

		pair->setValue(new JsonStringValue(this->srvhost));
		configObj->add(pair);
	}
	// port
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"port"));
		pair->setValue(new JsonNumericValue(this->port));
		configObj->add(pair);
	}

	return __STP_MV(configObj);
}

MiningConfig* MiningConfig::fromJson(const JsonObject *configObj) {
	MiningConfig* config = new MiningConfig(); __STP(config);

	const JsonValuePair* pair = configObj->get(L"address");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonStringValue* value = dynamic_cast<JsonStringValue*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"address of the Mining Config must be string type.", __FILE__, __LINE__);

		AddressDescriptor desc(value->getValue());
		config->setAddressDescriptor(&desc);
	}

	pair = configObj->get(L"type");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonStringValue* value = dynamic_cast<JsonStringValue*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"type of the Mining Config must be string type.", __FILE__, __LINE__);

		const UnicodeString* str = value->getValue();
		if(str->equals(L"pool")){
			config->setCalclatorType(TYPE_POOL);
		}else{
			config->setCalclatorType(TYPE_DEBUG);
		}
	}

	pair = configObj->get(L"protocol");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonStringValue* value = dynamic_cast<JsonStringValue*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"protocol of the Mining Config must be string type.", __FILE__, __LINE__);

		const UnicodeString* str = value->getValue();
		if(str->equals(L"ipv4")){
			config->setProtocol(P2pNodeRecord::TCP_IP_V4);
		}else{
			config->setProtocol(P2pNodeRecord::TCP_IP_V6);
		}
	}

	pair = configObj->get(L"host");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonStringValue* value = dynamic_cast<JsonStringValue*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"host of the Mining Config must be string type.", __FILE__, __LINE__);

		const UnicodeString* str = value->getValue();
		config->setServerHost(str);
	}

	pair = configObj->get(L"port");
	if(pair != nullptr){
		AbstractJsonObject* av = pair->getValue();
		JsonNumericValue* value = dynamic_cast<JsonNumericValue*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"port of the Mining Config must be numeric type.", __FILE__, __LINE__);

		config->setPort(value->getValue());
	}

	return __STP_MV(config);
}

} /* namespace codablecash */
