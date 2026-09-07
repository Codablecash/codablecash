/*
 * ZoneSimulator.cpp
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#include "sim/ZoneSimulator.h"
#include "sim/InstanceSimulator.h"

#include "base/StackRelease.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonArrayObject.h"
#include "json_object/JsonValuePair.h"
#include "json_object/JsonStringValue.h"


namespace codablecash {

ZoneSimulator::ZoneSimulator(uint16_t zone, INetworkSeeder* seeder) {
	this->zone = zone;
	this->seeder = seeder;
	this->list = new ArrayList<InstanceSimulator>();
}

ZoneSimulator::~ZoneSimulator() {
	shutdown();
}

void ZoneSimulator::shutdown() {
	if(this->list != nullptr){
		int maxLoop = this->list->size();
		for(int i = 0; i != maxLoop; ++i){
			InstanceSimulator* node = this->list->get(i);
			node->shutdown();
		}

		this->list->deleteElements();

		delete this->list, this->list = nullptr;
	}
}

JsonObject* ZoneSimulator::makeJsonSettingObject() {
	JsonObject* obj = new JsonObject(); __STP(obj);

	JsonArrayObject* instArray = new JsonArrayObject();
	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"zoneConfigs"));
		pair->setValue(instArray);
		obj->add(pair);
	}

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		InstanceSimulator* node = this->list->get(i);

		JsonObject* jsonObject = node->makeJsonSettingObject();
		instArray->add(jsonObject);
	}

	return __STP_MV(obj);
}

} /* namespace codablecash */
