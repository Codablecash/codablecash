/*
 * ZoneInstanceConfig.h
 *
 *  Created on: Sep 6, 2026
 *      Author: iizuka
 */

#ifndef SIM_ZONEINSTANCECONFIG_H_
#define SIM_ZONEINSTANCECONFIG_H_

namespace alinous {
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class GenesisBalanceConfig;
class MiningConfig;
class FinalizerConfig;

class ZoneInstanceConfig {
public:
	ZoneInstanceConfig(const ZoneInstanceConfig& inst);
	ZoneInstanceConfig();
	virtual ~ZoneInstanceConfig();

	void setGenesisConfig(const GenesisBalanceConfig* genConfig);
	void setMiningConfig(const MiningConfig* minerConfig);
	void setFinalizerConfig(const FinalizerConfig* fconfig);
	void setName(const UnicodeString* name);
	void setPort(int port);

private:
	GenesisBalanceConfig* genConfig;
	MiningConfig* minerConfig;
	FinalizerConfig* fconfig;
	UnicodeString* name;
	int port;
};

} /* namespace codablecash */

#endif /* SIM_ZONEINSTANCECONFIG_H_ */
