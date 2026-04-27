/*
 * BlockchainStatusCache.cpp
 *
 *  Created on: 2023/03/19
 *      Author: iizuka
 */

#include "bc_status_cache/BlockchainStatusCache.h"
#include "bc_status_cache/ZoneStatusCache.h"
#include "bc_status_cache/BlockHead.h"
#include "bc_status_cache/BlockIdChangedException.h"
#include "bc_status_cache_context/IStatusCacheContext.h"

#include "base_io/File.h"

#include "bc_base_conf_store/StatusStore.h"

#include "bc_block/Block.h"
#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"

#include "bc_block_body/BlockBody.h"

#include "pow/PoWManager.h"

#include "numeric/BigInteger.h"

#include "bc_memorypool/MemPoolTransaction.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_blockstore_body/BlockBodyStoreManager.h"

#include "base/ArrayList.h"
#include "base/StackRelease.h"

#include "bc_trx/AbstractBalanceTransaction.h"
#include "bc_trx/AbstractControlTransaction.h"

#include "bc_finalizer_pool/FinalizerPool.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_processor/CentralProcessor.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"

#include "bc_memorypool/MemoryPool.h"

#include "bc_status_cache/BlockHeadElement.h"

#include "bc_trx/AbstractInterChainCommunicationTansaction.h"

#include "transaction/AbstractSmartcontractTransaction.h"

#include "osenv/funcs.h"

#include "base_timestamp/SystemTimestamp.h"

#include "bc/CodablecashSystemParam.h"

namespace codablecash {

BlockchainStatusCache::BlockchainStatusCache(const File* baseDir, const CodablecashSystemParam* config, MemoryPool* memPool, const File* tmpCacheBaseDir, ISystemLogger* logger) {
	this->config = config;
	this->baseDir = new File(*baseDir);
	this->numZones = 0;
	this->zoneSelf = 0;
	this->statusStore = nullptr;
	this->powManager = nullptr;
	this->finalizer = nullptr;
	this->memberLock = new SysMutex();
	this->memPool = memPool;
	this->lastVoted = 0;
	this->logger = logger;
}

BlockchainStatusCache::~BlockchainStatusCache() {
	close();

	delete this->statusStore;
	delete this->baseDir;
	delete this->memberLock;

	this->memPool = nullptr;
}

void BlockchainStatusCache::initBlankCache(uint16_t zoneSelf, uint16_t numZones) {
	this->numZones = numZones;
	this->zoneSelf = zoneSelf;

	this->statusStore = new StatusStore(this->baseDir, CONFIG_FILE_NAME);
	saveConfig();

	for(int i = 0; i != numZones; ++i){
		ZoneStatusCache* cache = new ZoneStatusCache(this->baseDir, i, i != zoneSelf, this->logger, this->config);
		this->zoneList.addElement(cache);

		cache->initBlank();
	}
}

void BlockchainStatusCache::open() {
	this->statusStore = new StatusStore(this->baseDir, CONFIG_FILE_NAME);
	loadConfig();

	for(int i = 0; i != this->numZones; ++i){
		ZoneStatusCache* cache = new ZoneStatusCache(this->baseDir, this->logger, i != this->zoneSelf, this->config);
		this->zoneList.addElement(cache);

		cache->open();
	}
}

void BlockchainStatusCache::close() {
	int maxLoop = this->zoneList.size();
	for(int i = 0; i != maxLoop; ++i){
		ZoneStatusCache* cache = this->zoneList.get(i);
		cache->close();
	}

	this->zoneList.deleteElements();
	this->zoneList.reset();
}

void BlockchainStatusCache::initCacheStatus(CodablecashBlockchain *blockchain) {
	int maxLoop = this->zoneList.size();
	for(int i = 0; i != maxLoop; ++i){
		ZoneStatusCache* cache = this->zoneList.get(i);

		BlockHeaderStoreManager* headerStore = blockchain->getHeaderManager(i);
		if(!headerStore->isEmpty()){
			MemPoolTransaction* memTrx = this->memPool->begin(); __STP(memTrx);

			cache->updateBlockStatus(memTrx, blockchain, this->config);
		}
	}
}

void BlockchainStatusCache::saveConfig() {
	this->statusStore->addShortValue(KEY_NUM_ZONES, this->numZones);
	this->statusStore->addShortValue(KEY_ZONE_SELF, this->zoneSelf);
}

void BlockchainStatusCache::loadConfig() {
	this->statusStore->load();
	this->numZones = this->statusStore->getShortValue(KEY_NUM_ZONES);
	this->zoneSelf = this->statusStore->getShortValue(KEY_ZONE_SELF);
}

void BlockchainStatusCache::setPowManager(PoWManager *pow) noexcept {
	StackUnlocker __lock(this->memberLock, __FILE__, __LINE__);

	this->powManager = pow;
}

void BlockchainStatusCache::setFinalizer(FinalizerPool *finalizer) {
	StackUnlocker __lock(this->memberLock, __FILE__, __LINE__);

	this->finalizer = finalizer;
}

void BlockchainStatusCache::onBlockAdded(MemPoolTransaction* memTrx, const Block *block, CodablecashBlockchain *chain) {
	uint16_t zone = block->getZone();

	ZoneStatusCache* cache = this->zoneList.get(zone);
	cache->updateBlockStatus(memTrx, chain, this->config);
}

void BlockchainStatusCache::postBlockAdded(const Block *block, CodablecashBlockchain *chain) {
	uint16_t zone = block->getZone();
	ZoneStatusCache* cache = this->zoneList.get(zone);

	// check finalize
	int votePerBlock = this->config->getVotePerBlock();
	if(block->isFinalizing(votePerBlock)){
		reportFinalizing(zone, block, chain);
	}

	// report mining(PoW)
	{
		StackUnlocker __lock(this->memberLock, __FILE__, __LINE__);
		if(this->powManager != nullptr){
			report2PowManager(chain, cache);
		}
	}
}

void BlockchainStatusCache::onBlockHeaderAdded(MemPoolTransaction *memTrx, const BlockHeader *header, CodablecashBlockchain *chain) {
	uint16_t zone = header->getZone();

	ZoneStatusCache* cache = this->zoneList.get(zone);
	cache->updateBlockStatus(memTrx, chain, this->config);

	int votePerBlock = this->config->getVotePerBlock();
	if(header->isFinalizing(votePerBlock)){
		reportHeaderFinalizing(zone, header, chain);
	}
}

/*
void BlockchainStatusCache::postBlockHeaderAdded(const BlockHeader *block, CodablecashBlockchain *chain) {
}*/

void BlockchainStatusCache::reportHeaderFinalizing(uint16_t zone, const BlockHeader *header, CodablecashBlockchain *chain) const noexcept {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	uint64_t includingHeight = header->getHeight();
	const BlockHeaderId* id = header->getId();

	uint16_t voteBeforeNBlocks = this->config->getVoteBeforeNBlocks(includingHeight);
	uint16_t voteBlockIncludeAfterNBlocks = this->config->getVoteBlockIncludeAfterNBlocks(includingHeight);
	int beforeHeight = voteBeforeNBlocks + voteBlockIncludeAfterNBlocks;

	uint64_t finalizingHeight = includingHeight - beforeHeight;

	if(cache->getFinalizedHeight() < finalizingHeight){
		BlockHeaderStoreManager* headerManager = chain->getHeaderManager(zone);
		BlockHeader* finalizedheader = headerManager->getNBlocksBefore(id, includingHeight, beforeHeight); __STP(finalizedheader);

#ifdef __DEBUG__
		assert(finalizedheader != nullptr);
		assert(finalizingHeight == finalizedheader->getHeight());
#endif

		// Request Finalize
		CentralProcessor* processor = chain->getProcessor();
		processor->requestFinalizingHeader(zone, finalizingHeight, finalizedheader->getId());
	}
}

void BlockchainStatusCache::reportFinalizing(uint16_t zone, const Block *block, CodablecashBlockchain *chain) const noexcept {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	uint64_t includingHeight = block->getHeight();
	const BlockHeader* header = block->getHeader();
	const BlockHeaderId* id = header->getId();

	uint16_t voteBeforeNBlocks = this->config->getVoteBeforeNBlocks(includingHeight);
	uint16_t voteBlockIncludeAfterNBlocks = this->config->getVoteBlockIncludeAfterNBlocks(includingHeight);
	int beforeHeight = voteBeforeNBlocks + voteBlockIncludeAfterNBlocks;

	uint64_t finalizingHeight = includingHeight - beforeHeight;

	if(cache->getFinalizedHeight() < finalizingHeight){
		BlockHeaderStoreManager* headerManager = chain->getHeaderManager(zone);
		BlockHeader* finalizedheader = headerManager->getNBlocksBefore(id, includingHeight, beforeHeight); __STP(finalizedheader);

#ifdef __DEBUG__
		assert(finalizedheader != nullptr);
		assert(finalizingHeight == finalizedheader->getHeight());
#endif

		// Request Finalize
		CentralProcessor* processor = chain->getProcessor();
		processor->requestFinalizing(zone, finalizingHeight, finalizedheader->getId());
	}
}

void BlockchainStatusCache::report2PowManager(CodablecashBlockchain* blockchain, ZoneStatusCache *cache) noexcept {
	this->powManager->requestNewHeaderId();
}

BigInteger BlockchainStatusCache::calcTargetDiff(uint16_t zone, uint64_t height, const BlockHeaderId *headerId
		, const SystemTimestamp* tm, BlockHeaderStoreManager* headerManager) {
	ZoneStatusCache *cache = this->zoneList.get(zone);

	BlockHeader* header = headerManager->getHeader(headerId, height); __STP(header);

	assert(header != nullptr);

	return calcTargetDiff(headerManager, cache, header, tm);
}

BigInteger BlockchainStatusCache::calcTargetDiff(BlockHeaderStoreManager* headerManager, ZoneStatusCache *cache, const BlockHeader* header, const SystemTimestamp* end) {
	const BlockHeaderId* headerId = header->getId();

	uint64_t height = header->getHeight() + 1;
	uint16_t powHashrateBlocks = this->config->getPowHashrateBlocks(height);
	uint64_t powBlockTimeMills = this->config->getPowBlockTimeMills(height);


	return cache->calcTargetDifficulty(header, headerManager, powBlockTimeMills, powHashrateBlocks, end);
}

void BlockchainStatusCache::setScheduledBlock(uint16_t zone, const Block *block) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	cache->setScheduledBlock(block);
}

Block* BlockchainStatusCache::fetchScheduledBlock(uint16_t zone) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->fetchScheduledBlock();
}

Block* BlockchainStatusCache::getScheduledBlock(uint16_t zone) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->getScheduledBlock();
}

uint64_t BlockchainStatusCache::getHeadHeight(uint16_t zone) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	const BlockHead* head = cache->getHead();
	return head != nullptr ? head->getHeadHeight() : 0;
}

uint64_t BlockchainStatusCache::getFinalizedHeight(uint16_t zone) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->getFinalizedHeight();
}

uint64_t BlockchainStatusCache::getFinalizedTicketPrice(uint16_t zone) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->getTicketPrice();
}

const BlockHead* BlockchainStatusCache::getHead(uint16_t zone) const noexcept {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->getHead();
}

const BlockHead* BlockchainStatusCache::getSecondHead(uint16_t zone) const noexcept {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->getSecondHead();
}

BlockHeader* BlockchainStatusCache::changeMiningTarget(uint16_t zone) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->changeMiningTarget();
}

void BlockchainStatusCache::importCosumedMemTransactions(uint16_t zone,
		MemPoolTransaction *memTrx, uint64_t height, const BlockHeaderId *headerId, CodablecashBlockchain* blockchain) {
	ZoneStatusCache* cache = this->zoneList.get(zone);
	uint64_t finalizedHeight = cache->getFinalizedHeight();

	BlockHeaderStoreManager* headerManager = blockchain->getHeaderManager(zone);
	BlockBodyStoreManager* bodyStore = blockchain->getBlockBodyStoreManager(zone);

	ArrayList<BlockHeader> list;
	list.setDeleteOnExit();

	{
		uint64_t currentHeight = height;
		BlockHeaderId* currentHeaderId = dynamic_cast<BlockHeaderId*>(headerId->copyData());

		while(finalizedHeight < currentHeight){
			__STP(currentHeaderId);
			BlockHeader* header = headerManager->getHeader(currentHeaderId, currentHeight);
			list.addElement(header);

			currentHeaderId = dynamic_cast<BlockHeaderId*>(header->getLastHeaderId()->copyData());
			currentHeight--;
		}
		__STP(currentHeaderId);
	}


	int maxLoop = list.size();
	for(int i = 0; i != maxLoop; ++i){
		BlockHeader* blockHeader = list.get(i);
		const BlockMerkleRoot* merkleRoot = blockHeader->getMerkleRoot();

		BlockBody* body = bodyStore->getBlockBody(merkleRoot, blockHeader->getHeight()); __STP(body);
		if(body != nullptr){
			markConsumedTransactions(memTrx, body);
		}
	}
}

uint16_t BlockchainStatusCache::getNumZones(uint64_t height) const {
	return this->numZones;
}

void BlockchainStatusCache::markConsumedTransactions(MemPoolTransaction *memTrx, BlockBody *body) {
	{
		const ArrayList<AbstractControlTransaction>* list = body->getControlTransactions();
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractControlTransaction* trx = list->get(i);
			const TransactionId* trxId = trx->getTransactionId();

			memTrx->consume(trxId);
		}
	}
	{
		const ArrayList<AbstractInterChainCommunicationTansaction>* list = body->getInterChainCommunicationTransactions();
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractInterChainCommunicationTansaction* trx = list->get(i);
			const TransactionId* trxId = trx->getTransactionId();

			memTrx->consume(trxId);
		}
	}
	{
		const ArrayList<AbstractBalanceTransaction>* list = body->getBalanceTransactions();
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractBalanceTransaction* trx = list->get(i);
			const TransactionId* trxId = trx->getTransactionId();

			memTrx->consume(trxId);
		}
	}
	{
		const ArrayList<AbstractSmartcontractTransaction>* list = body->getSmartcontractTransactions();
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractSmartcontractTransaction* trx = list->get(i);
			const TransactionId* trxId = trx->getTransactionId();

			memTrx->consume(trxId);
		}
	}
}

void BlockchainStatusCache::updateFinalizedCacheData(uint16_t zone, uint64_t finalizingHeight
		, const BlockHeaderId *headerId, CodablecashBlockchain* blockchain, MemPoolTransaction* memtrx, IStatusCacheContext* context) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	cache->finalizeUpdateCacheData(finalizingHeight, headerId, blockchain, context);

	cache->updateBlockStatus(memtrx, blockchain, this->config);
}

void BlockchainStatusCache::updateFinalizedHeaderCacheData(uint16_t zone, uint64_t finalizingHeight, const BlockHeaderId *headerId
		, CodablecashBlockchain *blockchain, MemPoolTransaction* memTrx) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	cache->finalizeHeaderUpdateCacheData(finalizingHeight, headerId, blockchain);

	cache->updateBlockStatus(memTrx, blockchain, this->config);
}

UtxoData* BlockchainStatusCache::findUtxo(uint16_t zone, const UtxoId *utxoId) const {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->findUtxo(utxoId);
}

bool BlockchainStatusCache::registerBlockHeader4Limit(uint16_t zone, const BlockHeader *header, const CodablecashSystemParam *param) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->registerBlockHeader4Limit(header, param);
}

SystemTimestamp* BlockchainStatusCache::getPosVoteLimit(uint16_t zone, uint64_t lastHeight) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->getPosVoteLimit(lastHeight);
}

LockinManager* BlockchainStatusCache::getLockInManager(uint16_t zone) const noexcept {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	return cache->getLockinManager();
}

void BlockchainStatusCache::requestPosVote(uint16_t zone, uint64_t calculatedNonceHeight, CodablecashBlockchain *blockchain) {
	ZoneStatusCache* cache = this->zoneList.get(zone);

	const BlockHead* head = cache->getHead();
	const BlockHeader* header = head->getRealHeadHeader();
	const BlockHeaderId* id = header->getId();
	uint64_t height = header->getHeight();

	if(this->finalizer != nullptr && this->lastVoted < height){
		this->finalizer->requestCheckVoting(zone, id, height);
		this->lastVoted = height;
	}
}




} /* namespace codablecash */
