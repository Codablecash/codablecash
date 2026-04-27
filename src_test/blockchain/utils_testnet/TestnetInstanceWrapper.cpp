/*
 * TestnetInstanceWrapper.cpp
 *
 *  Created on: Nov 20, 2024
 *      Author: iizuka
 */

#include "TestnetInstanceWrapper.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_io/File.h"

#include "bc_network_instance/CodablecashNetworkNodeConfig.h"
#include "bc_network_instance/CodablecashNetworkNode.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"

#include "bc/CodablecashNodeInstance.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_p2p_info/P2pNodeRecord.h"
#include "IDebugSeeder.h"

namespace codablecash {

TestnetInstanceWrapper::TestnetInstanceWrapper(uint16_t zone, int index) {
	this->zone = zone;
	this->index = index;
	this->baseDir = nullptr;
	this->nwconfig = nullptr;
	this->logger = nullptr;
	this->node = nullptr;
}

TestnetInstanceWrapper::~TestnetInstanceWrapper() {
	if(this->node != nullptr){
		this->node->shutdown();
	}
	delete this->node;

	delete baseDir;
	delete nwconfig;
	this->logger = nullptr;
}

void TestnetInstanceWrapper::init(const wchar_t* name, int port, const File *zoneBaseDir, const CodablecashNetworkNodeConfig *baseNwconfig, ISystemLogger *logger) {
	this->logger = logger;

	UnicodeString seg(L"");
	seg.append((int64_t)this->index);

	this->baseDir = zoneBaseDir->get(&seg);

	this->nwconfig = new CodablecashNetworkNodeConfig(*baseNwconfig);
	this->nwconfig->setCanonicalName(name);
	this->nwconfig->setPort(port);

	NodeIdentifierSource* networkKey =  NodeIdentifierSource::create(); __STP(networkKey);
	this->nwconfig->setNetworkKey(networkKey);

	this->node = new CodablecashNetworkNode(this->baseDir, this->nwconfig, this->logger);
	this->node->setNodeName(name);
}

void TestnetInstanceWrapper::setGenesisBlockConfig(const GenesisBalanceConfig* gconfig) {
	this->nwconfig->setGenesisConfig(gconfig);
}

void TestnetInstanceWrapper::setMinerConfig(const MiningConfig *minerConfig) {
	this->nwconfig->setMinerConfig(minerConfig);
}

void TestnetInstanceWrapper::setFinalizerConfig(const FinalizerConfig *fconfig) {
	this->nwconfig->setFinalizerConfig(fconfig);
}

void TestnetInstanceWrapper::initGenesis() {
	this->node->setNetworkConfig(this->nwconfig);
	this->node->generateNetwork(this->zone);
}

void TestnetInstanceWrapper::initBlank() {
	this->node->setNetworkConfig(this->nwconfig);
	this->node->initBlank(this->zone);
}

void TestnetInstanceWrapper::start(IDebugSeeder* seeder, bool pending) {
	this->node->startNetwork(seeder, pending);

	this->node->startMiner();
	this->node->startStakePool();

	// register to seed
	{
		const NodeIdentifierSource* source = this->nwconfig->getNetworkKey();
		NodeIdentifier nodeId = source->toNodeIdentifier();

		const wchar_t* host = L"::1";
		int port = this->nwconfig->getPort();

		const UnicodeString* cname = this->nwconfig->getCanonicalName();

		P2pNodeRecord* rec = P2pNodeRecord::createIpV6Record(zone, &nodeId, cname, host, port); __STP(rec);
		seeder->addRecord(rec);
	}
}

void TestnetInstanceWrapper::syncNetwork() {
	this->node->syncNetwork();
}

uint64_t TestnetInstanceWrapper::getHeight() {
	CodablecashNodeInstance* inst = this->node->getInstance();

	BlockchainController* ctrl = inst->getController();
	uint64_t h = ctrl->getHeadHeight(this->zone);

	return h;
}

void TestnetInstanceWrapper::suspendMining() {
	this->node->suspendMining();
}

void TestnetInstanceWrapper::resumeMining() {
	this->node->resumeMining();
}

const FinalizerConfig* TestnetInstanceWrapper::getFinalizerConfig() const noexcept {
	return this->nwconfig->getFinalizerConfig();
}

} /* namespace codablecash */
