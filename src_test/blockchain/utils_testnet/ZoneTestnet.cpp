/*
 * ZoneTestnet.cpp
 *
 *  Created on: Nov 20, 2024
 *      Author: iizuka
 */

#include "ZoneTestnet.h"
#include "TestnetInstanceWrapper.h"

#include "base/UnicodeString.h"

#include "base_io/File.h"


namespace codablecash {

ZoneTestnet::ZoneTestnet(uint16_t zone, const File* projectFolder) {
	this->zone = zone;

	UnicodeString zoneStr(L"");
	zoneStr.append((int64_t)zone);
	this->zoneFolder = projectFolder->get(&zoneStr);

	if(!this->zoneFolder->exists()){
		this->zoneFolder->mkdirs();
	}
}

ZoneTestnet::~ZoneTestnet() {
	this->instances.deleteElements();

	delete this->zoneFolder;
}

TestnetInstanceWrapper* ZoneTestnet::createTestnetInstanceWrapper(const wchar_t* name, int port, const CodablecashNetworkNodeConfig* baseNwconfig, ISystemLogger* logger) {
	int index = this->instances.size();
	TestnetInstanceWrapper* inst = new TestnetInstanceWrapper(this->zone, index);
	inst->init(name, port, this->zoneFolder, baseNwconfig, logger);

	this->instances.addElement(inst);

	return inst;
}

void ZoneTestnet::startGenesis(IDebugSeeder* seeder) {
	TestnetInstanceWrapper* instWrapper = this->instances.get(0);

	instWrapper->initGenesis();
	instWrapper->start(seeder, false);
}

void ZoneTestnet::startFrom(IDebugSeeder *seeder, int fromIndex) {
	int maxLoop = this->instances.size();

	for(int i = fromIndex; i != maxLoop; ++i){
		TestnetInstanceWrapper* instWrapper = this->instances.get(i);

		instWrapper->initBlank();
		instWrapper->start(seeder, true);

		instWrapper->syncNetwork();
	}
}

TestnetInstanceWrapper* ZoneTestnet::getInstance(int index) const noexcept {
	return this->instances.get(index);
}

void ZoneTestnet::suspendMining() {
	int maxLoop = this->instances.size();
	for(int i = 0; i != maxLoop; ++i){
		TestnetInstanceWrapper* instWrapper = this->instances.get(i);

		instWrapper->suspendMining();
	}
}

void ZoneTestnet::resumeMining() {
	int maxLoop = this->instances.size();
	for(int i = 0; i != maxLoop; ++i){
		TestnetInstanceWrapper* instWrapper = this->instances.get(i);

		instWrapper->resumeMining();
	}
}

} /* namespace codablecash */
