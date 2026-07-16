/*
 * CodablecashNetworkInstance.cpp
 *
 *  Created on: 2023/08/21
 *      Author: iizuka
 */

#include "bc/CodablecashNodeInstance.h"
#include "bc/exceptions.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_trx_genesis/GenesisTransaction.h"

#include "base/StackRelease.h"

#include "base_io/File.h"

#include "bc_network_instance/CodablecashNetworkNode.h"
#include "bc_network_instance/CodablecashNetworkNodeConfig.h"
#include "bc_network_instance/GenesisBalanceConfig.h"
#include "bc_network_instance/INetworkSeeder.h"
#include "bc_network_instance/FinalizerConfig.h"

#include "bc_network_instance_sync/NetworkNodeSync.h"

#include "bc_block/Block.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_p2p_info/P2pNodeRecord.h"
#include "bc_p2p_info/P2pDnsManager.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "bc_memorypool/MemoryPool.h"
#include "bc_memorypool/MemPoolTransaction.h"

#include "bc_finalizer_pool/FinalizerPool.h"


namespace codablecash {

CodablecashNetworkNode::CodablecashNetworkNode(const File* baseDir, const CodablecashNetworkNodeConfig* nwconfig, ISystemLogger* logger) {
	this->baseDir = new File(*baseDir);
	this->nwconfig = new CodablecashNetworkNodeConfig(*nwconfig);
	this->inst = nullptr;
	this->logger = logger;
	this->nodeName = nullptr;
}

CodablecashNetworkNode::~CodablecashNetworkNode() {
	shutdown();
	delete this->baseDir;
	delete this->nwconfig;
	delete this->nodeName;
}

void CodablecashNetworkNode::shutdown() noexcept {
	if(this->inst != nullptr){
		this->inst->shutdown();
		delete this->inst;
		this->inst = nullptr;
	}
}

void CodablecashNetworkNode::setNetworkConfig(const CodablecashNetworkNodeConfig* nwconfig) {
	delete this->nwconfig;
	this->nwconfig = new CodablecashNetworkNodeConfig(*nwconfig);
}

bool CodablecashNetworkNode::initBlank(uint16_t zoneSelf, int numZones) {
	CodablecashNodeInstance inst(this->baseDir, this->logger, this->nwconfig->getSysConfig());
	bool bl = inst.initBlankInstance(zoneSelf, numZones);

	return bl;
}

bool CodablecashNetworkNode::generateNetwork(uint16_t zoneSelf, int numZones) {
	CodablecashNodeInstance inst(this->baseDir, this->logger, this->nwconfig->getSysConfig());
	bool bl = inst.initBlankInstance(zoneSelf, numZones);

	const GenesisBalanceConfig* gconfig = this->nwconfig->getGenesisConfig();
	bl = bl && (gconfig != nullptr);

	if(bl){
		CodablecashNodeInstance* inst = new CodablecashNodeInstance(this->baseDir, this->logger, this->nwconfig->getSysConfig()); __STP(inst);
		if(this->nodeName != nullptr){
			inst->setNodeName(this->nodeName);
		}
		inst->load();

		{
			GenesisTransaction* gtrx = new GenesisTransaction(); __STP(gtrx);

			const ArrayList<GenesisBalance>* list = gconfig->getList();

			int maxLoop = list->size();
			for(int i = 0; i != maxLoop; ++i){
				GenesisBalance* balance = list->get(i);

				const AddressDescriptor* addressDesc = balance->getAddressDescriptor();
				BalanceUnit amount(*balance->getBalanceUnit());

				BalanceUtxo utxo(amount);
				utxo.setAddress(addressDesc);

				gtrx->addBalanceUtxo(&utxo);
			}

			gtrx->build();

			Block block(0, 1);
			block.addBalanceTransaction(gtrx);
			block.build();

			BlockchainController* ctrl = inst->getController();
			ctrl->addBlock(&block);
		}

		inst->shutdown();
	}

	return bl;
}

void CodablecashNetworkNode::startNetwork(INetworkSeeder* seeder, bool pending) {
	this->inst = new CodablecashNodeInstance(this->baseDir, this->logger, this->nwconfig->getSysConfig());
	if(this->nodeName != nullptr){
		this->inst->setNodeName(this->nodeName);
	}

	this->inst->load();

	// init header selection
	this->inst->initCacheStatus();

	// startNetwork
	const UnicodeString* host = this->nwconfig->getHost();
	int port = this->nwconfig->getPort();

	this->inst->startNetwork(host, port);

	// start processors
	{
		const NodeIdentifierSource* networkKey = this->nwconfig->getNetworkKey();
		this->inst->startProcessors(networkKey, pending);
	}

	// init database
	if(seeder != nullptr){
		ArrayList<P2pNodeRecord>* seedList = seeder->getSeedNodes(); __STP(seedList);
		if(seedList != nullptr){
			seedList->setDeleteOnExit();
		}
		if(seedList != nullptr && !seedList->isEmpty()){
			initDatabase(seedList);
		}
	}

	// init connections
	maintainNetwork();
}

void CodablecashNetworkNode::maintainNetwork() {
	this->inst->maintainNetwork();
}

void CodablecashNetworkNode::initDatabase(ArrayList<P2pNodeRecord> *seedList) {
	P2pDnsManager* dnsManager = this->inst->getP2pDnsManager();
	int numZones = this->inst->getNumZones();

	int maxLoop = seedList->size();
	for(int i = 0; i != maxLoop; ++i){
		P2pNodeRecord* record = seedList->get(i);

		dnsManager->importSeeds(numZones, record, this->logger);
	}
}

void CodablecashNetworkNode::syncNetwork() {
	NetworkNodeSync sync(this);
	sync.sync();

	this->inst->resumeRequestProcessor();
}

void CodablecashNetworkNode::startMiner() {
	const MiningConfig* mconfig = this->nwconfig->getMinerConfig();

	if(mconfig != nullptr){
		this->inst->startBlockGenerator(mconfig);
		requestMining();
	}
}

void CodablecashNetworkNode::requestMining() {
	MemoryPool* memPool = this->inst->getMemoryPool();
	BlockchainController* ctrl = this->inst->getController();

	MemPoolTransaction* memTrx = memPool->begin(); __STP(memTrx);
	ctrl->requestMiningBlock(memTrx);
}

void CodablecashNetworkNode::suspendMining() {
	this->inst->suspendBlockGenerator();
}

void CodablecashNetworkNode::resumeMining() {
	this->inst->resumeBlockGenerator();
}


void CodablecashNetworkNode::startStakePool() {
	const FinalizerConfig* fconfig = this->nwconfig->getFinalizerConfig();

	if(fconfig != nullptr){
		const NodeIdentifierSource* voterSource = fconfig->getVoterSource();
		this->inst->startBlockFinalizer(voterSource);
	}

}

const NodeIdentifierSource* CodablecashNetworkNode::getVoterSource() const noexcept {
	FinalizerPool* finalizerPool = this->inst->getFinalizerPool();
	const NodeIdentifierSource* source = finalizerPool->getVoterIdentifierSource();

	return source;

}

void CodablecashNetworkNode::setNodeName(const wchar_t *name) noexcept {
	delete this->nodeName;
	this->nodeName = new UnicodeString(name);
}

uint64_t CodablecashNetworkNode::getMaxHeight(uint16_t zone) const {
	BlockchainController* ctrl = this->inst->getController();

	return ctrl->getHeadHeight(zone);
}

/**
 * use
 * StackHandshakeReleaser __releaser(handshake);
 * @param nodeId
 * @return
 */
BlockchainNodeHandshake* CodablecashNetworkNode::getNodeHandshakeByNodeId(const NodeIdentifier *nodeId) const noexcept {
	BlochchainP2pManager* p2pManager = this->inst->getBlochchainP2pManager();

	return p2pManager->getNodeHandshakeByNodeId(nodeId);
}

void CodablecashNetworkNode::broadCastShutdownCommand() {
	const NodeIdentifierSource* source = this->nwconfig->getNetworkKey();

	this->inst->broadCastShutdownCommand(source);
}

} /* namespace codablecash */
