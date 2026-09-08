/*
 * CodableSimNet.cpp
 *
 *  Created on: Sep 2, 2026
 *      Author: iizuka
 */

#include "sim/CodableSimNet.h"
#include "sim/ZoneSimulator.h"

#include "json_object/JsonObject.h"
#include "json_object/JsonArrayObject.h"
#include "json_object/JsonStringValue.h"
#include "json_object/JsonValuePair.h"

#include "base/StackRelease.h"


namespace codablecash {

CodableSimNet::CodableSimNet() {
	this->zones = new ArrayList<ZoneSimulator>();
}

CodableSimNet::~CodableSimNet() {
	int maxLoop = this->zones->size();
	for(int i = 0; i != maxLoop; ++i){
		ZoneSimulator* zsim = this->zones->get(i);
		zsim->shutdown();
	}

	this->zones->deleteElements();
	delete this->zones;
}

JsonObject* CodableSimNet::makeJsonSettingObject() {
	JsonObject* topObject = new JsonObject(); __STP(topObject);

	JsonArrayObject* zonesArray = new JsonArrayObject();

	{
		JsonValuePair* pair = new JsonValuePair();
		pair->setKey(new JsonStringValue(L"zoneConfigs"));
		pair->setValue(zonesArray);
		topObject->add(pair);
	}

	int maxLoop = this->zones->size();
	for(int i = 0; i != maxLoop; ++i){
		ZoneSimulator* zsim = this->zones->get(i);

		JsonObject* obj = zsim->makeJsonSettingObject();
		zonesArray->add(obj);
	}

	return __STP_MV(topObject);
}

} /* namespace codablecash */
