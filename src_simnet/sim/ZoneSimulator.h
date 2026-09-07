/*
 * ZoneSimulator.h
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#ifndef SIM_ZONESIMULATOR_H_
#define SIM_ZONESIMULATOR_H_
#include <cstdint>

#include "base/ArrayList.h"

using namespace alinous;

namespace codablecash {

class InstanceSimulator;
class INetworkSeeder;
class GenesisBalanceConfig;
class MiningConfig;
class FinalizerConfig;
class JsonObject;

class ZoneSimulator {
public:
	explicit ZoneSimulator(uint16_t zone, INetworkSeeder* seeder);
	virtual ~ZoneSimulator();

	void shutdown();

	void firstInstance(const GenesisBalanceConfig* gen, const MiningConfig* minerConfig, const FinalizerConfig* fconfig, const UnicodeString* name);

	JsonObject* makeJsonSettingObject();

private:
	INetworkSeeder* seeder;
	uint16_t zone;
	ArrayList<InstanceSimulator>* list;
};

} /* namespace codablecash */

#endif /* SIM_ZONESIMULATOR_H_ */
