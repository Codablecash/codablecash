/*
 * InstanceSimulator.h
 *
 *  Created on: Sep 6, 2026
 *      Author: iizuka
 */

#ifndef SIM_INSTANCESIMULATOR_H_
#define SIM_INSTANCESIMULATOR_H_

namespace codablecash {

class ZoneInstanceConfig;
class CodablecashNetworkNode;
class JsonObject;

class InstanceSimulator {
public:
	explicit InstanceSimulator(const ZoneInstanceConfig* config);
	virtual ~InstanceSimulator();

	void shutdown();

	JsonObject* makeJsonSettingObject();

private:
	ZoneInstanceConfig* config;
	CodablecashNetworkNode* netnode;
};

} /* namespace codablecash */

#endif /* SIM_INSTANCESIMULATOR_H_ */
