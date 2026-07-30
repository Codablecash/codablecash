/*
 * MultizoneTestnet.cpp
 *
 *  Created on: Nov 19, 2024
 *      Author: iizuka
 */

#include "MultizoneTestnet.h"
#include "ZoneTestnet.h"
#include "TestnetInstanceWrapper.h"

#include "../../test_utils/TestPortSelection.h"

#include "base_io/File.h"

#include "bc/CodablecashSystemParam.h"

#include "bc_network_instance/CodablecashNetworkNodeConfig.h"

#include "osenv/funcs.h"

#include "base/UnicodeString.h"

namespace codablecash {

MultizoneTestnet::MultizoneTestnet(StackTestPortGetter* portSel, const File* projectFolder, ISystemLogger* logger
		, const CodablecashSystemParam* param, const CodablecashNetworkNodeConfig* baseNwconfig) {
	this->portSel = portSel;
	this->projectFolder = new File(*projectFolder);
	this->logger = logger;
	this->param = new CodablecashSystemParam(*param);
	this->baseNwconfig = new CodablecashNetworkNodeConfig(*baseNwconfig);

	this->baseNwconfig->setSysConfig(this->param);
}

MultizoneTestnet::~MultizoneTestnet() {
	this->zones.deleteElements();

	delete this->projectFolder;
	delete this->param;
	delete this->baseNwconfig;
}

ZoneTestnet* MultizoneTestnet::getZone(uint16_t zone) {
	int size = this->zones.size();

	// add zone
	if(zone >= size){
		int maxLoop = zone + 1;
		for(int i = size; i < maxLoop; ++i){
			ZoneTestnet* zn = new ZoneTestnet(i, this->projectFolder);
			this->zones.addElement(zn);
		}
	}

	return this->zones.get(zone);
}

TestnetInstanceWrapper* MultizoneTestnet::createInstance(uint16_t zone, const wchar_t* name) {
	int port = this->portSel->allocPort();

	ZoneTestnet* zoneNet = getZone(zone);

	TestnetInstanceWrapper* instWrapper = zoneNet->createTestnetInstanceWrapper(name, port, this->baseNwconfig, this->logger);

	return instWrapper;
}

TestnetInstanceWrapper* MultizoneTestnet::getInstance(uint16_t zone, const wchar_t *name) {
	ZoneTestnet* zoneNet = getZone(zone);

	UnicodeString nameStr(name);
	TestnetInstanceWrapper* inst = zoneNet->getInstance(&nameStr);
	return inst;
}

void MultizoneTestnet::startGenesis(uint16_t zone, IDebugSeeder* seeder) {
	ZoneTestnet* zoneNet = getZone(zone);

	zoneNet->startGenesis(seeder);
}

void MultizoneTestnet::startInstancesFrom(uint16_t zone, int fromIndex, IDebugSeeder *seeder) {
	ZoneTestnet* zoneNet = getZone(zone);

	zoneNet->startFrom(seeder, fromIndex);
}

void MultizoneTestnet::waitForBlockHeight(uint16_t zone, int index, uint64_t height) {
	ZoneTestnet* zoneNet = getZone(zone);
	TestnetInstanceWrapper* inst = zoneNet->getInstance(index);

	uint64_t curHeight = inst->getHeight();
	while(curHeight <= height){
		Os::usleep(100 * 1000);
		curHeight = inst->getHeight();
	}
}

void MultizoneTestnet::waitForHeaderHeight(uint16_t zoneSelf, uint16_t zone, int index, uint64_t height) {
	ZoneTestnet* zoneNet = getZone(zoneSelf);

	TestnetInstanceWrapper* inst = zoneNet->getInstance(index);

}

void MultizoneTestnet::suspendMining(uint16_t zone) {
	ZoneTestnet* testnet = this->zones.get(zone);
	testnet->suspendMining();
}

void MultizoneTestnet::resumeMining(uint16_t zone) {
	ZoneTestnet* testnet = this->zones.get(zone);
	testnet->resumeMining();
}



} /* namespace codablecash */
