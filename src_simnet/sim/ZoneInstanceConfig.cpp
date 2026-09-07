/*
 * ZoneInstanceConfig.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: iizuka
 */

#include "sim/ZoneInstanceConfig.h"

#include "bc_block_generator/MiningConfig.h"

#include "base/UnicodeString.h"

#include "bc_network_instance/FinalizerConfig.h"
#include "bc_network_instance/GenesisBalanceConfig.h"


namespace codablecash {

ZoneInstanceConfig::ZoneInstanceConfig(const ZoneInstanceConfig &inst) {
	this->genConfig = inst.genConfig != nullptr ? new GenesisBalanceConfig(*inst.genConfig) : nullptr;

}

ZoneInstanceConfig::ZoneInstanceConfig() {
	this->genConfig = nullptr;
	this->minerConfig = nullptr;
	this->fconfig = nullptr;
	this->name = nullptr;
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

void ZoneInstanceConfig::setPort(int port) {
	this->port = port;
}

} /* namespace codablecash */
