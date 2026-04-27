/*
 * BlockchainController.cpp
 *
 *  Created on: 2023/03/21
 *      Author: iizuka
 */

#include "bc_status_cache/BlockchainController.h"
#include "bc_status_cache/BlockchainStatusCache.h"
#include "bc_status_cache/ZoneStatusCache.h"
#include "bc_status_cache/BlockHeadElement.h"

#include "bc_status_cache_context/StatusCacheContext.h"
#include "bc_status_cache_context/CachedStatusCacheRepository.h"
#include "bc_status_cache_context/CachedStatusCacheContext.h"

#include "bc_status_cache_context_finalizer/AlreadyFinalizedException.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_blockstore_body/BlockBodyStoreManager.h"

#include "base_thread/ConcurrentGate.h"

#include "base_io/File.h"

#include "base/StackRelease.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/BlockHeader.h"
#include "bc_block/Block.h"

#include "bc_block_body/BlockBody.h"

#include "bc_status_cache/BlockHead.h"

#include "bc_memorypool/MemPoolTransaction.h"
#include "bc_memorypool/MemoryPool.h"
#include "bc_memorypool/FeeBananceTransactionScanner.h"
#include "bc_memorypool/ControlTransactionScanner.h"
#include "bc_memorypool/SmartcontractTansactionScanner.h"
#include "bc_memorypool/InterChainCommunicationTransactionScanner.h"

#include "bc/ISystemLogger.h"

#include "numeric/BigInteger.h"

#include "bc_block_vote/VotePart.h"

#include "bc_trx/AbstractBlockchainTransaction.h"
#include "bc_trx/AbstractBalanceTransaction.h"
#include "bc_trx/AbstractControlTransaction.h"
#include "bc_trx/AbstractInterChainCommunicationTansaction.h"

#include "transaction/AbstractSmartcontractTransaction.h"


namespace codablecash {

BlockchainController::BlockchainController(ISystemLogger* logger, const CodablecashSystemParam* config, const File* tmpCacheBaseDir, CodablecashBlockchain* blockchain
		, BlockchainStatusCache* statusCache, MemoryPool* memoryPool) {
	this->logger = logger;
	this->config = config;
	this->tmpCacheBaseDir = new File(*tmpCacheBaseDir);
	this->blockchain = blockchain;
	this->statusCache = statusCache;

	this->blockchain->addListner(this->statusCache);

	this->rwLock = new ConcurrentGate();

	this->statusCacheRepo = new CachedStatusCacheRepository(tmpCacheBaseDir);
	this->memoryPool = memoryPool;
}

BlockchainController::~BlockchainController() {
	delete this->tmpCacheBaseDir;
	this->blockchain = nullptr;
	this->statusCache = nullptr;

	delete this->statusCache;

	delete this->statusCacheRepo;

	delete this->rwLock;
	this->memoryPool = nullptr;
}

bool BlockchainController::addBlock(const Block *block) {
	{
		BlockHeader* header = block->getHeader();

		UnicodeString* strId = header->getId()->toString(); __STP(strId);

		UnicodeString message(L"[Add Block] Height: ");
		message.append((int64_t)header->getHeight());
		message.append(L" Header Id: ").append(strId);

		this->logger->log(&message);

		{
			uint16_t zone = header->getZone();
			registerBlockHeader4Limit(zone, header, this->config);
		}
	}

	{
		MemPoolTransaction* memTrx = this->memoryPool->begin(); __STP(memTrx);
		{
			StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

			{
				BlockHeader* header = block->getHeader();
				uint16_t zone = header->getZone();
				uint64_t height = header->getHeight();
				const BlockHeaderId* headerId = header->getId();

				BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(zone);
				BlockHeader* h = headerManager->getHeader(headerId, height); __STP(h);
				if(h != nullptr){
					return false;
				}
			}

			this->blockchain->addBlock(memTrx, block);

			// it must execute here, because finalizer updates head status
			this->blockchain->firePostBlockAdded(block);
		}
	}

	return true;
}

bool BlockchainController::addBlockHeader(const BlockHeader *header) {
	{
		UnicodeString* strId = header->getId()->toString(); __STP(strId);

		UnicodeString message(L"[Add Block Header] Height: ");
		message.append((int64_t)header->getHeight());
		message.append(L" Header Id: ").append(strId);

		this->logger->log(&message);

		{
			uint16_t zone = header->getZone();
			registerBlockHeader4Limit(zone, header, this->config);
		}
	}

	MemPoolTransaction* memTrx = this->memoryPool->begin(); __STP(memTrx);
	{
		StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

		{
			uint16_t zone = header->getZone();
			uint64_t height = header->getHeight();
			const BlockHeaderId* headerId = header->getLastHeaderId();

			BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(zone);
			BlockHeader* h = headerManager->getHeader(headerId, height); __STP(h);
			if(h != nullptr){
				return false;
			}
		}

		this->blockchain->addHeader(memTrx, header);
	}

	return true;
}

uint64_t BlockchainController::getHeadHeight(uint16_t zone) {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	return this->statusCache->getHeadHeight(zone);
}

uint16_t BlockchainController::getZoneSelf() const noexcept {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	return this->blockchain->getZoneSelf();
}

uint64_t BlockchainController::getFinalizedHeight(uint16_t zone) const {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	return this->statusCache->getFinalizedHeight(zone);
}

void BlockchainController::importCosumedMemTransactions(uint16_t zone,
		MemPoolTransaction *memTrx, uint64_t height, const BlockHeaderId *headerId) {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	this->statusCache->importCosumedMemTransactions(zone, memTrx, height, headerId, this->blockchain);
}

void BlockchainController::stopMining() noexcept {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	this->statusCache->setPowManager(nullptr);
}

void BlockchainController::stopFinalizing() noexcept {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	this->statusCache->setFinalizer(nullptr);
}

IStatusCacheContext* BlockchainController::getStatusCacheContext(uint16_t zone, const BlockHeaderId *headerId, uint64_t height) {
	StackWriteLock lock(this->rwLock, __FILE__, __LINE__);

	return doGetStatusCacheContext(zone, headerId, height, &lock);
}

IStatusCacheContext* BlockchainController::getStatusCacheContext(uint16_t zone) {
	StackWriteLock lock(this->rwLock, __FILE__, __LINE__);

	const BlockHead* head = this->statusCache->getHead(zone);
	const BlockHeader* header = head->getHeadHeader();

	uint64_t height = header->getHeight();
	const BlockHeaderId *headerId = header->getId();

	return doGetStatusCacheContext(zone, headerId, height, &lock);
}

IStatusCacheContext* BlockchainController::getBlankStatusCacheContext(uint16_t zone) {
	StackWriteLock lock(this->rwLock, __FILE__, __LINE__);

	StatusCacheContext* context = new StatusCacheContext(this->config, this->tmpCacheBaseDir
						, zone, lock.move(), this->statusCache, this->blockchain); __STP(context);
	context->init();

	return __STP_MV(context);
}

IStatusCacheContext* BlockchainController::doGetStatusCacheContext(uint16_t zone, const BlockHeaderId *headerId, uint64_t height, StackWriteLock* lock) {
	{
		CachedStatusCache* cache = this->statusCacheRepo->getCache(headerId);
		if(cache != nullptr){
			CachedStatusCacheContext* context = new CachedStatusCacheContext(cache, this->config
					, this->tmpCacheBaseDir, zone, lock->move(), this->statusCache, this->blockchain); __STP(context);
			context->init();

			context->loadInitialVotersData(); // load Initial Voters Data
			context->setTopHeight(height);

			return __STP_MV(context);
		}
	}

	// get cached context
	StatusCacheContext* context = nullptr;
	{
		uint64_t finalizedHeight = this->statusCache->getFinalizedHeight(zone);
		BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(zone);

		uint64_t currentHeight = height;
		BlockHeaderId* currentHeaderId = dynamic_cast<BlockHeaderId*>(headerId->copyData());

		while(finalizedHeight < currentHeight){
			__STP(currentHeaderId);

			BlockHeader* header = headerManager->getHeader(currentHeaderId, currentHeight);
			// if the header is scheduled block
			if(header == nullptr){
				Block* scblock = __getScheduledBlock(); __STP(scblock);
				assert(scblock != nullptr);

				const BlockHeader* scHeader = scblock->getHeader();
				header = dynamic_cast<BlockHeader*>(scHeader->copyData());
			}
			__STP(header);

			assert(header != nullptr);

			// check cache
			CachedStatusCache* cachedStatus = this->statusCacheRepo->getCache(currentHeaderId);
			if(cachedStatus != nullptr){
				context = new CachedStatusCacheContext(cachedStatus, this->config
								, this->tmpCacheBaseDir, zone, lock->move(), this->statusCache, this->blockchain);

				delete __STP_MV(currentHeaderId);
				currentHeaderId = nullptr;

				break;
			}

			// for next loop
			{
				const BlockHeaderId* lastId = header->getLastHeaderId();
				currentHeaderId = lastId != nullptr ? dynamic_cast<BlockHeaderId*>(lastId->copyData()) : nullptr;
			}
			currentHeight--;
		}
		delete currentHeaderId;
	}

	if(context == nullptr){
		context = new StatusCacheContext(this->config, this->tmpCacheBaseDir
							, zone, lock->move(), this->statusCache, this->blockchain);
	}
	__STP(context);

	initCacehContext(zone, context, headerId, height);
	this->statusCacheRepo->registerCache(height, headerId, context);

	return __STP_MV(context);
}

void BlockchainController::initCacehContext(uint16_t zone, StatusCacheContext *context,	const BlockHeaderId *headerId, uint64_t height) {
	context->init();

	uint64_t analyzedHeight = context->getPreAnalyzedHeight(); // finalyzed or cached
	BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(zone);
	BlockBodyStoreManager* bodyManager = this->blockchain->getBlockBodyStoreManager(zone);

	ArrayList<BlockHeader> list;
	list.setDeleteOnExit();

	uint64_t currentHeight = height;
	BlockHeaderId* currentHeaderId = dynamic_cast<BlockHeaderId*>(headerId->copyData());

	while(analyzedHeight < currentHeight){
		__STP(currentHeaderId);

		BlockHeader* header = headerManager->getHeader(currentHeaderId, currentHeight);
		if(header == nullptr){
			Block* scblock = __getScheduledBlock(); __STP(scblock);
			assert(scblock != nullptr);

			const BlockHeader* scHeader = scblock->getHeader();
			header = dynamic_cast<BlockHeader*>(scHeader->copyData());
		}

		assert(header != nullptr);
		__STP(header);

		list.addElement(__STP_MV(header));

		{
			const BlockHeaderId* lastId = header->getLastHeaderId();
			currentHeaderId = lastId != nullptr ? dynamic_cast<BlockHeaderId*>(lastId->copyData()) : nullptr;
		}
		currentHeight--;
	}
	delete currentHeaderId;

	// load voters
	// StatusCacheContext -> from finalized cache
	// CachedStatusCacheContext -> from cached data
	context->loadInitialVotersData();

	int maxLoop = list.size();
	for(int i = maxLoop - 1; i >= 0 ; --i){
		BlockHeader* header = list.get(i);
		currentHeight = header->getHeight();

		const BlockMerkleRoot* root = header->getMerkleRoot();
		BlockBody* blockBody = bodyManager->getBlockBody(root, currentHeight);
		if(blockBody == nullptr){
			Block* scblock = __getScheduledBlock(); __STP(scblock);
			assert(scblock != nullptr);

			const BlockBody* scBody = scblock->getBody();
			blockBody = dynamic_cast<BlockBody*>(scBody->copyData());
		}
		__STP(blockBody);

		assert(blockBody != nullptr);

		context->importBlock(header, blockBody, this->logger);
	}

	// set Top height
	context->setTopHeight(height);
}

/*
BlockHeader* BlockchainController::getNblocksBefore(uint16_t zone, const BlockHeaderId *headerId, uint64_t height, int voteBeforeNBlocks) const {
	StackReadLock __lock(this->rwLock);

	return __getNblocksBefore(zone, headerId, height, voteBeforeNBlocks);
}*/

BlockHeader* BlockchainController::__getNblocksBefore(uint16_t zone,
		const BlockHeaderId *headerId, uint64_t height, int voteBeforeNBlocks) const {
	BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(zone);

	return headerManager->getNBlocksBefore(headerId, height, voteBeforeNBlocks);
}

void BlockchainController::finalize(uint16_t zone, uint64_t finalizingHeight, const BlockHeaderId *headerId, MemoryPool* memPool) {
	{
		UnicodeString* strId = headerId->toString(); __STP(strId);

		UnicodeString message(L"  [Finalize] Height: ");
		message.append((int64_t)finalizingHeight);
		message.append(L" Header Id: ").append(strId);

		this->logger->log(&message);
	}

	// check double finalize -> throw exception
	{
		uint64_t finalizedHeight = getFinalizedHeight(zone);
		BlockchainController::checkFinalizedHeight(finalizedHeight, finalizingHeight);
	}

	{
		// lock memory at first
		MemPoolTransaction* memTrx = memPool->begin(); __STP(memTrx);

		// lock by context
		IStatusCacheContext* context = getBlankStatusCacheContext(zone); __STP(context);

		// clean blockchain
		uint64_t lastFinalizedHeight = this->statusCache->getFinalizedHeight(zone);
		this->blockchain->cleanOnFinalize(zone, finalizingHeight, headerId, lastFinalizedHeight);

		// update finalized data
		this->statusCache->updateFinalizedCacheData(zone, finalizingHeight, headerId, this->blockchain, memTrx, context);

		// clean memory Pool
		{
			BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(zone);
			BlockHeader* header = headerManager->getHeader(headerId, finalizingHeight); __STP(header);
			memTrx->cleanOnFinalize(context, finalizingHeight, header, lastFinalizedHeight);
		}

		// clear context cache
		context->close();
		this->statusCacheRepo->clear();
	}
}

void BlockchainController::finalizeHeader(uint16_t zone, uint64_t finalizingHeight, const BlockHeaderId *headerId, MemoryPool* memPool) {
	{
		UnicodeString* strId = headerId->toString(); __STP(strId);

		UnicodeString message(L"  [Finalize Header] Height: ");
		message.append((int64_t)finalizingHeight);
		message.append(L" Header Id: ").append(strId);

		this->logger->log(&message);
	}

	// check double finalize -> throw exception
	{
		uint64_t finalizedHeight = getFinalizedHeight(zone);
		BlockchainController::checkFinalizedHeight(finalizedHeight, finalizingHeight);
	}

	{
		// lock memory at first
		MemPoolTransaction* memTrx = memPool->begin(); __STP(memTrx);

		StackWriteLock lock(this->rwLock, __FILE__, __LINE__);

		// clean blockchain
		uint64_t lastFinalizedHeight = this->statusCache->getFinalizedHeight(zone);
		this->blockchain->cleanOnFinalize(zone, finalizingHeight, headerId, lastFinalizedHeight);

		// set finalized height
		// update finalized data
		this->statusCache->updateFinalizedHeaderCacheData(zone, finalizingHeight, headerId, this->blockchain, memTrx);
	}

	// header does not use status cache
}

void BlockchainController::checkFinalizedHeight(uint64_t finalizedHeight, uint64_t finalizingHeight) {
	if(finalizedHeight >= finalizingHeight){
		throw new AlreadyFinalizedException(__FILE__, __LINE__);
	}
}

int BlockchainController::getMempoolTrxCount() const noexcept {
	int count = 0;
	{
		MemPoolTransaction* memTrx = this->memoryPool->begin(); __STP(memTrx);
		count = memTrx->getTransactionsCount();
	}

	return count;
}

ArrayList<AbstractBlockchainTransaction>* BlockchainController::fetchMempoolTrx(const ArrayList<BloomFilter1024> *filters) {
	ArrayList<AbstractBlockchainTransaction>* ret = new ArrayList<AbstractBlockchainTransaction>(); __STP(ret);

	MemPoolTransaction* memTrx = this->memoryPool->begin(); __STP(memTrx);

	{
		ControlTransactionScanner* scanner = memTrx->beginScanControlTransaction(); __STP(scanner);
		while(scanner->hasNext()){
			AbstractControlTransaction* trx = scanner->next(); __STP(trx);
			if(trx->checkFilter(filters)){
				ret->addElement(__STP_MV(trx));
			}
		}
	}

	{
		FeeBananceTransactionScanner* scanner = memTrx->beginScanBalanceTransaction(); __STP(scanner);
		while(scanner->hasNext()){
			AbstractBalanceTransaction* trx = scanner->next(); __STP(trx);
			if(trx->checkFilter(filters)){
				ret->addElement(__STP_MV(trx));
			}
		}
	}

	{
		InterChainCommunicationTransactionScanner* scanner = memTrx->beginScanInterChainCommunicationTransaction(); __STP(scanner);
		while(scanner->hasNext()){
			AbstractInterChainCommunicationTansaction* trx = scanner->next(); __STP(trx);
			if(trx->checkFilter(filters)){
				ret->addElement(__STP_MV(trx));
			}
		}
	}

	{
		SmartcontractTansactionScanner* scanner = memTrx->beginScanSmartcontractTansaction(); __STP(scanner);
		while(scanner->hasNext()){
			AbstractSmartcontractTransaction* trx = scanner->next(); __STP(trx);
			if(trx->checkFilter(filters)){
				ret->addElement(__STP_MV(trx));
			}
		}
	}


	return __STP_MV(ret);
}

void BlockchainController::getSyncHeaderData(uint16_t zone, uint64_t offsetHeight, int limit, IBlockDetectCallback* callback) {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	uint64_t height = offsetHeight;
	for(int i = 0; i != limit; ++i){
		ArrayList<Block>* list = this->blockchain->getBlocksHeightAt(zone, height); __STP(list);
		if(list == nullptr){
			break;
		}
		list->setDeleteOnExit();

		callback->detect(height, list);

		height++;
	}

}

ArrayList<Block>* BlockchainController::getBlocksHeightAt(uint16_t zone, uint64_t height) const {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	return this->blockchain->getBlocksHeightAt(zone, height);
}

Block* BlockchainController::getBlockHeightAt(uint16_t zone, uint64_t height, const BlockHeaderId *headerId) const {
	ArrayList<Block>* list = getBlocksHeightAt(zone, height); __STP(list);
	if(list != nullptr){
		list->setDeleteOnExit();
	}

	return list != nullptr ? getBlockById(list, headerId) : nullptr;
}

Block* BlockchainController::getBlockById(ArrayList<Block> *list, const BlockHeaderId *headerId) {
	Block* ret = nullptr;

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const Block* block = list->get(i);
		const BlockHeader* header = block->getHeader();
		const BlockHeaderId* id = header->getId();

		if(headerId->equals(id)){
			ret = new Block(*block);
			break;
		}
	}

	return ret;
}

bool BlockchainController::hasHeaderId(uint16_t zone, uint64_t height, const BlockHeaderId *headerId) const {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(zone);
	BlockHeader* header = headerManager->getHeader(headerId, height); __STP(header);

	return header != nullptr;
}

BigInteger BlockchainController::calcTargetDifficulty(uint16_t zone, uint64_t lastHeight, const BlockHeaderId *lastHeaderId, const SystemTimestamp* tm) {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(zone);

	return this->statusCache->calcTargetDiff(zone, lastHeight, lastHeaderId, tm, headerManager);
}

void BlockchainController::requestMiningBlock(MemPoolTransaction* memTrx) {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	uint16_t zoneSelf = this->blockchain->getZoneSelf();
	ZoneStatusCache* cache = this->statusCache->getZoneStatusCache(zoneSelf);

	cache->updateBlockStatus(memTrx, this->blockchain, this->config);

	this->statusCache->report2PowManager(this->blockchain, cache);
}

const VoterEntry* BlockchainController::getVoterEntry(const NodeIdentifier *nodeId) {
	uint16_t zone = this->blockchain->getZoneSelf();

	IStatusCacheContext* context = getStatusCacheContext(zone); __STP(context);

	const VoterEntry* entry = context->getVoterEntry(nodeId);
	return entry;
}

BlockHeader* BlockchainController::getTopHeader() const {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	uint16_t zoneSelf = this->blockchain->getZoneSelf();
	const BlockHead* head = this->statusCache->getHead(zoneSelf);

	const BlockHeader* header = head->getHeadHeader();
	return dynamic_cast<BlockHeader*>(header->copyData());
}

bool BlockchainController::checkAcceptSecondRealBlockOnMining() const {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	bool result = false;

	uint16_t zoneSelf = this->blockchain->getZoneSelf();

	const BlockHead* head = this->statusCache->getHead(zoneSelf); // scheduled block
	const BlockHead* secondHead = this->statusCache->getSecondHead(zoneSelf); // real block

	uint64_t topHeight = head != nullptr ? head->getHeadHeader()->getHeight() : 0;
	uint64_t secondHeight = secondHead != nullptr ? secondHead->getHeadHeader()->getHeight() : 0;


	if(head != nullptr && secondHead != nullptr && (topHeight == secondHeight)){
		const BlockHeader* header = head->getHeadHeader();
		const BlockHeader* secondHeader = secondHead->getHeadHeader();

		const BlockHeaderId* headerId = header->getLastHeaderId();
		const BlockHeaderId* secondHeaderId = secondHeader->getLastHeaderId();

		if(headerId->equals(secondHeaderId)){
			const BlockHeadElement* element = head->getHeadBlockHead();
			const BlockHeadElement* secondElement = secondHead->getHeadBlockHead();

			int mevScore = element->getMevHealthScore();
			int secondMevScore = secondElement->getMevHealthScore();

			int powAllowedSceduledDiffMevScore = this->config->getPowAllowedSceduledDiffMevScore();

			result = ((mevScore - secondMevScore) < powAllowedSceduledDiffMevScore);
		}
	}

	return result;
}

BlockHeader* BlockchainController::changeMiningTarget() {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	uint16_t zoneSelf = this->blockchain->getZoneSelf();

	return this->statusCache->changeMiningTarget(zoneSelf);
}

void BlockchainController::setScheduledBlock(const Block *block) {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	uint16_t zoneSelf = this->blockchain->getZoneSelf();
	this->statusCache->setScheduledBlock(zoneSelf, block);
}

Block* BlockchainController::fetechScheduledBlock() {
	StackReadLock __lock(this->rwLock, __FILE__, __LINE__);

	return __fetechScheduledBlock();
}

Block* BlockchainController::__fetechScheduledBlock() {
	uint16_t zoneSelf = this->blockchain->getZoneSelf();
	return this->statusCache->fetchScheduledBlock(zoneSelf);
}

Block* BlockchainController::__getScheduledBlock() {
	uint16_t zoneSelf = this->blockchain->getZoneSelf();
	return this->statusCache->getScheduledBlock(zoneSelf);
}

bool BlockchainController::registerBlockHeader4Limit(uint16_t zone,	const BlockHeader *header, const CodablecashSystemParam *param) {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	return this->statusCache->registerBlockHeader4Limit(zone, header, param);
}

SystemTimestamp* BlockchainController::getPosVoteLimit(uint16_t zone, uint64_t lastHeight) {
	StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

	return this->statusCache->getPosVoteLimit(zone, lastHeight);
}

void BlockchainController::requestPosVote(uint16_t zone, uint64_t calculatedNonceHeight) {
	this->statusCache->requestPosVote(zone, calculatedNonceHeight, this->blockchain);
}

} /* namespace codablecash */
