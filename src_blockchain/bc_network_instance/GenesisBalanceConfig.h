/*
 * GenesisBalanceConfig.h
 *
 *  Created on: 2023/08/21
 *      Author: iizuka
 */

#ifndef BC_NETWORK_INSTANCE_GENESISBALANCECONFIG_H_
#define BC_NETWORK_INSTANCE_GENESISBALANCECONFIG_H_

#include "bc_base/BalanceUnit.h"

#include "base/ArrayList.h"

namespace codablecash {

class AddressDescriptor;
class JsonObject;

class GenesisBalance {
public:
	GenesisBalance(const GenesisBalance& inst);
	GenesisBalance();
	~GenesisBalance();

	void init(const BalanceUnit amount, const AddressDescriptor* desc) noexcept;

	const BalanceUnit* getBalanceUnit() const noexcept {
		return &this->amount;
	}
	const AddressDescriptor* getAddressDescriptor() const noexcept {
		return this->addressDesc;
	}

	JsonObject* toJsonObject() const;
	static GenesisBalance* fromJson(const JsonObject* balanceObj);

private:
	BalanceUnit amount;
	AddressDescriptor* addressDesc;
};

class GenesisBalanceConfig {
public:
	GenesisBalanceConfig(const GenesisBalanceConfig& inst);
	GenesisBalanceConfig();
	virtual ~GenesisBalanceConfig();

	void addBalance(const BalanceUnit amount, const AddressDescriptor* desc) noexcept;
	void addBalance(const GenesisBalance* balance);

	const ArrayList<GenesisBalance>* getList() const noexcept {
		return this->list;
	}

	JsonObject* toJsonObject() const;
	static GenesisBalanceConfig* fromJson(const JsonObject* configObj);

private:
	ArrayList<GenesisBalance>* list;
};

} /* namespace codablecash */

#endif /* BC_NETWORK_INSTANCE_GENESISBALANCECONFIG_H_ */
