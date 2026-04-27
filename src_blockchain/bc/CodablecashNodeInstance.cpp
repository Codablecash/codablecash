/*
 * CodablecashNodeInstance.cpp
 *
 *  Created on: 2023/03/04
 *      Author: iizuka
 */

#include "bc/CodablecashNodeInstance.h"
#include "bc/DefaultInstanceMemberAllocator.h"
#include "bc/ISystemLogger.h"

#include "base/StackRelease.h"
#include "base/Exception.h"

#include "base_io/File.h"

#include "p2pserver/P2pServer.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_memorypool/MemoryPool.h"

#include "bc_status_cache/BlockchainStatusCache.h"
#include "bc_status_cache/BlockchainController.h"

#include "pow/PoWManager.h"

#include "bc_block_generator/BlockGenerator.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_p2p_processor/P2pRequestProcessor.h"

#include "bc_p2p/BlochchainP2pManager.h"

#include "bc_finalizer_pool/FinalizerPool.h"

#include "bc_p2p_info/P2pDnsManager.h"
#include "bc_p2p_info/P2pNodeRecord.h"

#include "pubsub/PubSubId.h"
#include "pubsub/P2pHandshake.h"

#include "bc_p2p_cmd/LoginPubSubCommand.h"
#include "bc_p2p_cmd/P2pHandshakeAuthenticationException.h"
#include "pubsub_cmd/AbstractCommandResponse.h"

#include "bc_network/NodeIdentifierSource.h"
#include "bc_network/NodeIdentifier.h"


#include "base/ArrayList.h"

#include "bc/CodablecashSystemParam.h"
#include "bc/ExceptionThrower.h"

#include "bc_p2p_selector/ConnectNodeSelector.h"

#include "numeric/BigInteger.h"

#include "bc_blockstore/InvalidZoneException.h"

#include "bc_p2p_cmd_network/NodeShutdownCommand.h"

#include "base_timestamp/SystemTimestamp.h"

#include "bc_p2p_cmd_network/NodeNetworkInfo.h"
namespace codablecash {

CodablecashNodeInstance::CodablecashNodeInstance(const File* baseDir, ISystemLogger* logger, const CodablecashSystemParam* param) {
	DefaultInstanceMemberAllocator defalloc;
	this->allocator = defalloc.copy();

	__init(baseDir, logger, param);
}

CodablecashNodeInstance::CodablecashNodeInstance(const File *baseDir,	ISystemLogger *logger, const CodablecashSystemParam *param,
		const IInstanceMemberAllocator *allocator) {
	this->allocator = allocator->copy();

	__init(baseDir, logger, param);

}

void CodablecashNodeInstance::__init(const File *baseDir, ISystemLogger *logger, const CodablecashSystemParam *param) {
	this->param = new CodablecashSystemParam(*param);

	this->centralProcessor = nullptr;

	this->logger = logger;
	this->baseDir = new File(*baseDir);

	this->blockchain = nullptr;
	this->memoryPool = nullptr;
	this->statusCache = nullptr;
	this->ctrl = nullptr;

	this->powManager = nullptr;
	this->blockGenerator = nullptr;

	this->finalizer = nullptr;

	this->p2pServer = nullptr;
	this->p2pManager = nullptr;
	this->p2pRequestProcessor = nullptr;
	this->p2pDnsManager = nullptr;
	this->nodeName = nullptr;
}

CodablecashNodeInstance::~CodablecashNodeInstance() {
	shutdown();

	delete this->p2pServer;
	delete this->baseDir;
	delete this->param;

	delete this->allocator;
	delete this->nodeName;
}

bool CodablecashNodeInstance::initBlankInstance(uint16_t zoneSelf, uint16_t numZones) {
	try{
		File* blockChainDir = this->baseDir->get(BLOCKCHAIN_DIR); __STP(blockChainDir);
		File* memoryPoolBaseDir = this->baseDir->get(MEMORY_POOL_DIR); __STP(memoryPoolBaseDir);
		File* statusCacheBaseDir = this->baseDir->get(STATUS_CACHE_DIR); __STP(statusCacheBaseDir);
		File* tmpCacheBaseDir = this->baseDir->get(TMP_CACHE_DIR); __STP(tmpCacheBaseDir); // managed by ctrl

		CodablecashBlockchain::createBlankBlockchain(blockChainDir, zoneSelf, numZones);

		MemoryPool pool(memoryPoolBaseDir);
		pool.createBlankPool();

		BlockchainStatusCache cache(statusCacheBaseDir, this->param, &pool, tmpCacheBaseDir, this->logger);
		cache.initBlankCache(zoneSelf, numZones);

		P2pDnsManager p2pDatabase(this->baseDir);
		p2pDatabase.createBlankDatabase();

		P2pRequestProcessor requestProcessor(this->baseDir, this->p2pManager, this->p2pDnsManager, this->param, this->logger);
		requestProcessor.createBlank(this);
	}
	catch(Exception* e){
		this->logger->logException(e);
		delete e;
		return false;
	}

	return true;
}

void CodablecashNodeInstance::load() {
	File* blockChainDir = this->baseDir->get(BLOCKCHAIN_DIR); __STP(blockChainDir);
	File* memoryPoolBaseDir = this->baseDir->get(MEMORY_POOL_DIR); __STP(memoryPoolBaseDir);
	File* statusCacheBaseDir = this->baseDir->get(STATUS_CACHE_DIR); __STP(statusCacheBaseDir);
	File* tmpCacheBaseDir = this->baseDir->get(TMP_CACHE_DIR); __STP(tmpCacheBaseDir);

	this->blockchain = new CodablecashBlockchain(blockChainDir);
	this->blockchain->open();

	this->memoryPool = new MemoryPool(memoryPoolBaseDir);
	this->memoryPool->open();

	this->statusCache = new BlockchainStatusCache(statusCacheBaseDir, this->param, this->memoryPool, tmpCacheBaseDir, this->logger);
	this->statusCache->open();

	this->ctrl = new BlockchainController(this->logger, this->param, tmpCacheBaseDir, this->blockchain, this->statusCache, this->memoryPool);

	this->centralProcessor = new CentralProcessor(this->param, this->memoryPool, this->ctrl, this->logger);
	this->centralProcessor->start(this->nodeName);

	this->blockchain->setProcessor(this->centralProcessor);

	this->p2pDnsManager = new P2pDnsManager(this->baseDir);
	this->p2pDnsManager->open();
}

void CodablecashNodeInstance::initCacheStatus() {
	this->statusCache->initCacheStatus(this->blockchain);
}

void CodablecashNodeInstance::startNetwork(const UnicodeString *host, int port) {
	this->p2pServer = new P2pServer(this->logger, this);
	this->p2pManager = new BlochchainP2pManager();

	this->p2pManager->init(this->blockchain->getNumZones());

	this->p2pServer->addConnectionListener(this->p2pManager);

	this->p2pServer->startIpV6Listening(host, port, this->nodeName);
	this->p2pManager->setPort(port);
	this->p2pManager->setProtocol(P2pNodeRecord::TCP_IP_V6);
	this->p2pManager->setHost(host);

	this->centralProcessor->setBlochchainP2pManager(this->p2pManager);
}

void CodablecashNodeInstance::startProcessors(const NodeIdentifierSource *networkKey, bool suspend) {
	if(this->p2pRequestProcessor == nullptr){
		this->p2pRequestProcessor = new P2pRequestProcessor(this->baseDir, this->p2pManager, this->p2pDnsManager, this->param, this->logger);
		if(this->nodeName != nullptr){
			this->p2pRequestProcessor->setNodeName(this->nodeName);
		}

		this->p2pRequestProcessor->open(networkKey, this, suspend);

		this->centralProcessor->setP2pRequestProcessor(this->p2pRequestProcessor);
	}
}

void CodablecashNodeInstance::shutdownNetwork() {
	if(this->p2pServer != nullptr){
		this->p2pServer->stopListning();
		this->p2pManager->endCommunication();

		this->p2pManager->resetZones();
		this->p2pServer->dispose();

		delete this->p2pManager;
		this->p2pManager = nullptr;

		delete this->p2pServer;
		this->p2pServer = nullptr;
	}

	if(this->p2pRequestProcessor != nullptr){
		this->p2pRequestProcessor->close();
	}

	delete this->p2pRequestProcessor;
	this->p2pRequestProcessor = nullptr;

}

void CodablecashNodeInstance::startBlockGenerator(const MiningConfig *config) {
	uint16_t zone = this->blockchain->getZoneSelf();

	this->powManager = new PoWManager(this->logger, config);
	this->blockGenerator = this->allocator->newBlockGenerator(zone, this->param, this->memoryPool, this->ctrl, config, this->logger);
	this->blockGenerator->setCentralProcessor(this->centralProcessor);

	this->powManager->setBlockGenerator(this->blockGenerator);

	this->statusCache->setPowManager(this->powManager);

	this->powManager->start();
}

void CodablecashNodeInstance::startBlockFinalizer(const NodeIdentifierSource *nodeSource) {
	this->finalizer = this->allocator->newFinalizerPool(this->param, this->memoryPool, this->ctrl
			, this->p2pRequestProcessor, this->p2pManager, this->logger);
	this->finalizer->setVoterIdentifier(nodeSource);

	this->statusCache->setFinalizer(this->finalizer);

	this->finalizer->start();
}

void CodablecashNodeInstance::shutdown() {
	if(this->p2pServer != nullptr){ // stop network acception
		this->p2pServer->stopListning();
		this->p2pManager->endCommunication();
	}

	if(this->powManager != nullptr){
		this->ctrl->stopMining();
		this->powManager->shutdown();
	}

	if(this->finalizer != nullptr){
		this->ctrl->stopFinalizing();
		this->finalizer->shutdown();
	}

	if(this->centralProcessor != nullptr){
		this->centralProcessor->shutdown();
	}

	shutdownNetwork();

	if(this->blockchain != nullptr){
		this->blockchain->close();
		delete this->blockchain, this->blockchain = nullptr;
	}
	if(this->memoryPool != nullptr){
		this->memoryPool->close();
		delete this->memoryPool, this->memoryPool = nullptr;
	}
	if(this->statusCache != nullptr){
		this->statusCache->close();
		delete this->statusCache, this->statusCache = nullptr;
	}
	if(this->p2pDnsManager != nullptr){
		this->p2pDnsManager->close();
		delete this->p2pDnsManager;
		this->p2pDnsManager = nullptr;
	}

	delete this->powManager;
	this->powManager = nullptr;

	delete this->finalizer;
	this->finalizer = nullptr;

	delete this->blockGenerator;
	this->blockGenerator = nullptr;

	delete this->centralProcessor;
	this->centralProcessor = nullptr;

	delete this->ctrl;
	this->ctrl = nullptr;

}

void CodablecashNodeInstance::suspendBlockGenerator() {
	if(this->powManager != nullptr){
		this->powManager->suspend();
	}
}

void CodablecashNodeInstance::resumeBlockGenerator() {
	if(this->powManager != nullptr){
		this->powManager->resume();
	}
}

bool CodablecashNodeInstance::isBlockGeneratorSuspendStatus() const noexcept {
	return this->powManager->isSuspendStatus();
}

void CodablecashNodeInstance::connectIpV6Node(uint16_t zone, const UnicodeString *host, int port, const NodeIdentifier* nodeId, const UnicodeString* canonicalName) {
	ERROR_POINT(L"CodablecashNodeInstance::connectIpV6Node");

	doConnectIpNode(zone, host, port, IP_V6, nodeId, canonicalName);
}

void CodablecashNodeInstance::connectIpV4Node(uint16_t zone, const UnicodeString *host, int port, const NodeIdentifier* nodeId, const UnicodeString* canonicalName) {
	ERROR_POINT(L"CodablecashNodeInstance::connectIpV4Node");

	doConnectIpNode(zone, host, port, IP_V4, nodeId, canonicalName);
}

void CodablecashNodeInstance::doConnectIpNode(uint16_t zone, const UnicodeString* host, int port, int protocol, const NodeIdentifier* nodeId, const UnicodeString* canonicalName){
	PubSubId* psId = PubSubId::createNewId(); __STP(psId);
	P2pHandshake *handshake = new P2pHandshake(psId, this->logger); __STP(handshake);
	handshake->init(this->p2pServer->getConnectionManager(), this);

	// connect
	if(protocol == IP_V4){
		handshake->connectIpV4(host, port, handshake, false);
	}else{
		handshake->connectIpV6(host, port, handshake, false);
	}

	//  login
	loginNode(zone, handshake, canonicalName);

	// register connection
	this->p2pServer->registerHandshake(__STP_MV(handshake));

	// register node
	try{
		//NodeIdentifierSource* source = this->p2pRequestProcessor->getNetworkKey();
		//NodeIdentifier nodeId = source->toNodeIdentifier();
		this->p2pManager->registerHandshake(zone, handshake, nodeId, canonicalName);
	}
	catch(Exception* e){
		handshake->dispose();
		throw e;
	}

	// start subscribe
	handshake->startSubscribe();
}

void CodablecashNodeInstance::loginNode(uint16_t zone, P2pHandshake *handshake, const UnicodeString* canonicalName) {
	// login
	NodeIdentifierSource* source = this->p2pRequestProcessor->getNetworkKey();

	uint16_t zoneSelf = this->blockchain->getZoneSelf();
	LoginPubSubCommand cmd(zoneSelf, canonicalName);

	{
		NodeIdentifier nodeId = source->toNodeIdentifier();
		SystemTimestamp tm;

		int protocol = this->p2pManager->getProtocol();
		const UnicodeString* host = this->p2pManager->getHost();
		uint32_t port = this->p2pManager->getPort();

		NodeNetworkInfo info(zoneSelf, &nodeId, &tm, protocol, host, port);
		info.setCanonicalName(this->nodeName);

		cmd.setNodeNetworkInfo(&info);
	}

	cmd.sign(source);

	AbstractCommandResponse* response = handshake->publishCommand(&cmd); __STP(response);
	if(response != nullptr){
		UnicodeString* msg = response->toString(); __STP(msg);
		this->logger->debugLog(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE, msg, __FILE__, __LINE__);
	}

	uint32_t type = response->getType();
	if(type != AbstractCommandResponse::TYPE_RES_OK){
		throw new P2pHandshakeAuthenticationException(L"Already connected.", __FILE__, __LINE__);
	}
}

NodeIdentifierSource* CodablecashNodeInstance::getNetworkKey() const noexcept  {
	return this->p2pRequestProcessor->getNetworkKey();
}

uint16_t CodablecashNodeInstance::getZoneSelf() const noexcept {
	return this->blockchain->getZoneSelf();
}

int CodablecashNodeInstance::getNumZones() const noexcept {
	return this->blockchain->getNumZones();
}

void CodablecashNodeInstance::maintainNetwork() {
	int numZones = getNumZones();

	for(int i = 0; i != numZones; ++i){
		__maintainNetwork(i);
	}
}

void CodablecashNodeInstance::__maintainNetwork(uint16_t zone) {
	int numNeedConnection = this->param->getNumConnectionExtShard();
	if(zone == getZoneSelf()){
		numNeedConnection = this->param->getNumConnectionOwnShard();
	}

	int currentSize = this->p2pManager->getNumNodes(zone);
	int diff = numNeedConnection - currentSize;

	if(diff > 0){
		ArrayList<P2pNodeRecord>* list = this->p2pDnsManager->getZoneRecords(zone, 1024); __STP(list);
		list->setDeleteOnExit();

		ConnectNodeSelector selector;
		selector.importCandidates(list);

		while(diff > 0 && selector.hasNext()){
			const P2pNodeRecord* rec = selector.fetchNext();

			if(__connectWithP2pNodeRecord(rec)){
				diff--;
			}
		}
	}
}

bool CodablecashNodeInstance::__connectWithP2pNodeRecord(const P2pNodeRecord *record) {
	try{
		int protocol = record->getProtocol();
		const UnicodeString* host = record->getHost();
		int port = record->getPort();
		uint16_t zone = record->getZone();

		const NodeIdentifier* nodeId = record->getNodeIdentifier();
		const UnicodeString* canonicalName = record->getCanonicalName();

		if(protocol == P2pNodeRecord::TCP_IP_V4){
			connectIpV4Node(zone, host, port, nodeId, canonicalName);
		}
		else if(protocol == P2pNodeRecord::TCP_IP_V6){
			connectIpV6Node(zone, host, port, nodeId, canonicalName);
		}
	}
	catch(Exception* e){
		__STP(e);

		return false;
	}

	return true;
}

void CodablecashNodeInstance::resumeRequestProcessor() {
	this->p2pRequestProcessor->resume(this);
}

File* CodablecashNodeInstance::getTempCacheDir() const {
	return this->baseDir->get(TMP_CACHE_DIR);
}

void CodablecashNodeInstance::setNodeName(const UnicodeString *name) noexcept {
	delete this->nodeName;
	this->nodeName = new UnicodeString(name);
}

void CodablecashNodeInstance::validateZone(uint16_t zone) const {
	uint16_t numZone = this->blockchain->getNumZones();

	ExceptionThrower<InvalidZoneException>::throwExceptionIfCondition(numZone <= zone, L"", __FILE__, __LINE__);
}

void CodablecashNodeInstance::broadCastShutdownCommand(const NodeIdentifierSource *source) {
	NodeShutdownCommand command;

	NodeIdentifier nodeId = source->toNodeIdentifier();

	command.signTimestamp(source);
	command.sign(source);

	this->p2pManager->broadCastAllZones(nullptr, &command, this->p2pRequestProcessor);
}

} /* namespace codablecash */
