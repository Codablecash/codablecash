/*
 * InstanceSimulator.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: iizuka
 */

#include "sim/InstanceSimulator.h"
#include "sim/ZoneInstanceConfig.h"

#include "bc_network_instance/CodablecashNetworkNode.h"

#include "base/StackRelease.h"

#include "json_object/JsonObject.h"

#include "json_object/JsonValuePair.h"

#include "json_object/JsonStringValue.h"
namespace codablecash {

InstanceSimulator::InstanceSimulator(const ZoneInstanceConfig* config) {
	this->netnode = nullptr;
	this->config = new ZoneInstanceConfig(*config);
}

InstanceSimulator::~InstanceSimulator() {
	shutdown();
}

void InstanceSimulator::shutdown() {
	if(this->netnode != nullptr){
		this->netnode->shutdown();
		delete this->netnode, this->netnode = nullptr;
	}
}

JsonObject* InstanceSimulator::makeJsonSettingObject() {
	JsonObject* obj = new JsonObject(); __STP(obj);

	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L""));
	}


	return __STP_MV(obj);
	// FIXME
}

} /* namespace codablecash */
