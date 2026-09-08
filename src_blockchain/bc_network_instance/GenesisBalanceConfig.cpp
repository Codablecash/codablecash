/*
 * GenesisBalanceConfig.cpp
 *
 *  Created on: 2023/08/21
 *      Author: iizuka
 */

#include "bc_network_instance/GenesisBalanceConfig.h"

#include "bc_base/AddressDescriptor.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonStringValue.h"
#include "json_object/JsonArrayObject.h"
#include "json_object/JsonNumericValue.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "bc/ExceptionThrower.h"

#include "sim/SimnetConfigException.h"

#include "bc_base/BalanceUnit.h"
namespace codablecash {

GenesisBalance::GenesisBalance(const GenesisBalance &inst) : amount(inst.amount) {
	this->addressDesc = inst.addressDesc != nullptr ? dynamic_cast<AddressDescriptor*>(inst.addressDesc->copyData()) : nullptr;
}

GenesisBalance::GenesisBalance() : amount(0L) {
	this->addressDesc = nullptr;
}

GenesisBalance::~GenesisBalance() {
	delete this->addressDesc;
}

void GenesisBalance::init(const BalanceUnit amount, const AddressDescriptor *desc) noexcept {
	this->amount = amount;

	delete this->addressDesc;
	this->addressDesc = new AddressDescriptor(*desc);
}

JsonObject* GenesisBalance::toJsonObject() const {
	JsonObject* configObj = new JsonObject(); __STP(configObj);

	// address
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"address"));

		const char* cstr = this->addressDesc->toCString();
		StackArrayRelease<const char> __cstr(cstr);

		UnicodeString str(cstr);
		pair->setValue(new JsonStringValue(&str));

		configObj->add(pair);
	}
	// balance
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"balance"));

		uint64_t amout = this->amount.getAmount();
		pair->setValue(new JsonNumericValue(amout));

		configObj->add(pair);
	}

	return __STP_MV(configObj);
}

GenesisBalance* GenesisBalance::fromJson(const JsonObject *balanceObj) {
	GenesisBalance* balance = new GenesisBalance(); __STP(balance);

	const JsonValuePair* apair = balanceObj->get(L"address");
	const JsonValuePair* bpair = balanceObj->get(L"balance");
	if(apair != nullptr && bpair != nullptr){
		AbstractJsonObject* av = apair->getValue();
		JsonStringValue* avalue = dynamic_cast<JsonStringValue*>(av);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(avalue == nullptr, L"address of the balance must be string type.", __FILE__, __LINE__);

		AbstractJsonObject* bv = bpair->getValue();
		JsonNumericValue* bvalue = dynamic_cast<JsonNumericValue*>(bv);
		ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(bvalue == nullptr, L"amount of the balance must be numeric type.", __FILE__, __LINE__);

		// address desc
		const UnicodeString* str = avalue->getValue();
		AddressDescriptor* desc = new AddressDescriptor(str); __STP(desc);

		// amount
		BalanceUnit amount((uint64_t)bvalue->getValue());

		balance->init(amount, desc);
	}

	return __STP_MV(balance);
}

GenesisBalanceConfig::GenesisBalanceConfig(const GenesisBalanceConfig &inst) {
	this->list = new ArrayList<GenesisBalance>();

	int maxLoop = inst.list->size();
	for(int i = 0; i != maxLoop; ++i){
		GenesisBalance* balance = inst.list->get(i);

		this->list->addElement(new GenesisBalance(*balance));
	}
}

GenesisBalanceConfig::GenesisBalanceConfig() {
	this->list = new ArrayList<GenesisBalance>();
}

GenesisBalanceConfig::~GenesisBalanceConfig() {
	this->list->deleteElements();
	delete this->list;
}

void GenesisBalanceConfig::addBalance(const BalanceUnit amount, const AddressDescriptor *desc) noexcept {
	GenesisBalance* bl = new GenesisBalance();
	bl->init(amount, desc);

	this->list->addElement(bl);
}

void GenesisBalanceConfig::addBalance(const GenesisBalance *balance) {
	this->list->addElement(new GenesisBalance(*balance));
}

JsonObject* GenesisBalanceConfig::toJsonObject() const {
	JsonObject* configObj = new JsonObject(); __STP(configObj);

	JsonArrayObject* balances = new JsonArrayObject();
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"balances"));
		pair->setValue(balances);
		configObj->add(pair);
	}

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		GenesisBalance* balance = this->list->get(i);

		balances->add(balance->toJsonObject());
	}

	return __STP_MV(configObj);
}



GenesisBalanceConfig* GenesisBalanceConfig::fromJson(const JsonObject* configObj) {
	GenesisBalanceConfig* config = new GenesisBalanceConfig(); __STP(config);

	const JsonValuePair* pair = configObj->get(L"balances");
	AbstractJsonObject* __av = pair->getValue();
	JsonArrayObject* value = dynamic_cast<JsonArrayObject*>(__av);
	ExceptionThrower<SimnetConfigException>::throwExceptionIfCondition(value == nullptr, L"balances of the Genesis Balance Config must be array type.", __FILE__, __LINE__);

	int maxLoop = value->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractJsonObject* av = value->get(i);
		JsonObject* balanceObj = dynamic_cast<JsonObject*>(av);

		GenesisBalance* balance = GenesisBalance::fromJson(balanceObj); __STP(balance);
		config->addBalance(balance);
	}

	return __STP_MV(config);
}

} /* namespace codablecash */
