/*
 * ZoneStatusCache.cpp
 *
 *  Created on: 2023/03/19
 *      Author: iizuka
 */

#include "bc_status_cache/ZoneStatusCache.h"
#include "bc_status_cache/HeadBlockDetector.h"
#include "bc_status_cache/BlockHead.h"

#include "base_io/File.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "bc_base_conf_store/StatusStore.h"

#include "numeric/BigInteger.h"

#include "pow/PoWNonce.h"

#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_blockstore_body/BlockBodyStoreManager.h"

#include "pow/PoWNonceResult.h"

#include "bc_block_body/BlockBody.h"

#include "bc_status_cache_data/FinalizedDataCache.h"

#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc_status_cache_context_finalizer/VoterStatusCacheContext.h"

#include "base_timestamp/SystemTimestamp.h"
#include "bc/CodablecashSystemParam.h"

#include "bc_status_cache_lockin/LockinManager.h"

#include "bc_status_cache_vote/VoteManager.h"


namespace codablecash {

ZoneStatusCache::ZoneStatusCache(const File* baseDir, uint16_t zone, bool headerOnly, ISystemLogger* logger, const CodablecashSystemParam* config) {
	this->zone = zone;
	this->headerOnly = headerOnly;

	this->finalizedHeight = 0;

	UnicodeString name(DIR_NAME_BASE);
	addIdex2String(&name);
	this->baseDir = baseDir->get(&name);

	this->headBlockDetector = new HeadBlockDetector(logger);
	this->statusStore = nullptr;

	this->finalizedCache = new FinalizedDataCache(this->baseDir);
	this->lockinManager = new LockinManager(this->baseDir);

	this->voteManager = new VoteManager(this->baseDir, config);

	this->ticketPrice = config->getTicketPriceDefault(1L);
}

ZoneStatusCache::ZoneStatusCache(const File *baseDir, ISystemLogger* logger, bool headerOnly, const CodablecashSystemParam* config) {
	this->zone = 0;
	this->headerOnly = headerOnly;
	this->finalizedHeight = 0;

	UnicodeString name(DIR_NAME_BASE);
	addIdex2String(&name);
	this->baseDir = baseDir->get(&name);

	this->headBlockDetector = new HeadBlockDetector(logger);
	this->statusStore = nullptr;

	this->finalizedCache = new FinalizedDataCache(this->baseDir);
	this->lockinManager = new LockinManager(this->baseDir);
	this->voteManager = new VoteManager(this->baseDir, config);

	this->ticketPrice = 0;
}

ZoneStatusCache::~ZoneStatusCache() {
	close();

	delete this->headBlockDetector;
	delete this->baseDir;
}

void ZoneStatusCache::addIdex2String(UnicodeString *str) const noexcept {
	char* buff = new char[32];
	Mem::memset(buff, 0, 32);

	StackArrayRelease<char> __ST_buff(buff);

	::sprintf(buff, "%08d", this->zone);
	UnicodeString buffstr(buff);
	str->append(&buffstr);
}

void ZoneStatusCache::initBlank() {
	this->baseDir->mkdirs();

	this->statusStore = new StatusStore(this->baseDir, STATUS_STORE_FILE_NAME);
	saveStatus();

	this->finalizedCache->initBlank();
	this->lockinManager->initBlank();
	this->voteManager->initBlank();
}

void ZoneStatusCache::open() {
	this->statusStore = new StatusStore(this->baseDir, STATUS_STORE_FILE_NAME);
	loadStatus();

	this->finalizedCache->open();
	this->lockinManager->open();
	this->voteManager->open();
}

void ZoneStatusCache::close() {
	if(this->statusStore != nullptr){
		this->statusStore->close();
		delete this->statusStore;
		this->statusStore = nullptr;
	}

	if(this->finalizedCache != nullptr){
		delete this->finalizedCache;
		this->finalizedCache = nullptr;
	}

	if(this->lockinManager != nullptr){
		this->lockinManager->close();
		delete this->lockinManager;
		this->lockinManager = nullptr;
	}

	if(this->voteManager != nullptr){
		this->voteManager->close();
		delete this->voteManager;
		this->voteManager = nullptr;
	}
}

void ZoneStatusCache::saveStatus() {
	this->statusStore->addShortValue(KEY_ZONE, this->zone);
	this->statusStore->addShortValue(KEY_HEADER_ONLY, this->headerOnly ? 1 : 0);
	this->statusStore->addLongValue(KEY_FINALIZED_HEIGHT, this->finalizedHeight);
	this->statusStore->addLongValue(KEY_FINALIZED_TICKET_PRICE, this->ticketPrice);
}

void ZoneStatusCache::loadStatus() {
	this->statusStore->load();

	this->zone = this->statusStore->getShortValue(KEY_ZONE);
	uint16_t bl = this->statusStore->getShortValue(KEY_HEADER_ONLY);
	this->headerOnly = (bl == 1);
	this->finalizedHeight = this->statusStore->getLongValue(KEY_FINALIZED_HEIGHT);
	this->ticketPrice = this->statusStore->getLongValue(KEY_FINALIZED_TICKET_PRICE);
}

void ZoneStatusCache::updateBlockStatus(MemPoolTransaction* memTrx, CodablecashBlockchain *chain, const CodablecashSystemParam* config) {
	this->headBlockDetector->reset();
	// Make header lines
	this->headBlockDetector->buildHeads(this->zone, chain, this->finalizedHeight);

	// evaluate
	this->headBlockDetector->evaluate(this->zone, memTrx, chain, config, this->headerOnly);
	this->headBlockDetector->selectChain();
}

void ZoneStatusCache::finalizeUpdateCacheData(uint64_t finalizingHeight, const BlockHeaderId *headerId
		, CodablecashBlockchain *blockchain, IStatusCacheContext* context) {
	uint64_t lastFinalizedHeight = this->finalizedHeight;

	ArrayList<BlockHeaderId> list;
	list.setDeleteOnExit();

	BlockHeaderStoreManager* headerManager = blockchain->getHeaderManager(this->zone);
	BlockBodyStoreManager* bodyManager = blockchain->getBlockBodyStoreManager(this->zone);

	// make list
	{
		uint64_t height = finalizingHeight;
		BlockHeaderId* currentHeaderId = dynamic_cast<BlockHeaderId*>(headerId->copyData());

		while(height > lastFinalizedHeight){
			__STP(currentHeaderId);
			list.addElement(dynamic_cast<BlockHeaderId*>(currentHeaderId->copyData()));

			BlockHeader* header = headerManager->getHeader(currentHeaderId, height); __STP(header);

			currentHeaderId = dynamic_cast<BlockHeaderId*>(header->getLastHeaderId()->copyData());
			height--;
		}
		__STP(currentHeaderId);
	}

	context->loadInitialVotersData();

	// analyze blocks
	{
		uint64_t height = lastFinalizedHeight + 1;

		int startLoop = list.size() - 1;
		for(int i = startLoop; i >= 0; --i){
			BlockHeaderId* id = list.get(i);

			BlockHeader* header = headerManager->getHeader(id, height); __STP(header);
			assert(header != nullptr);
			const BlockMerkleRoot* root = header->getMerkleRoot();

			BlockBody* body = bodyManager->getBlockBody(root, height); __STP(body);

			this->finalizedCache->importBlockData(finalizingHeight, header, body, context);

			height++;
		}
	}

	// writeback Voter
	this->finalizedCache->writeBackVoterEntries(context);
	this->finalizedCache->writeBackVoterStatus(context);

	this->finalizedHeight = finalizingHeight;
	this->ticketPrice = context->getTicketPrice();
	saveStatus();

	// clean
	{
		const CodablecashSystemParam* params = context->getConfig();
		VoterStatusCacheContext* voterCache = this->finalizedCache->getVotingStatusCache();

		uint16_t voteBlockIncludeAfterNBlocks = params->getVoteBlockIncludeAfterNBlocks(this->finalizedHeight);
		uint16_t voteBeforeNBlocks = params->getVoteBeforeNBlocks(this->finalizedHeight);
		// uint16_t voteLockinIntervalHeight = params->getVoteLockinIntervalHeight();

		uint64_t cleanHeight = finalizingHeight - voteBlockIncludeAfterNBlocks - voteBeforeNBlocks;

		voterCache->clean(cleanHeight);
	}

}

void ZoneStatusCache::finalizeHeaderUpdateCacheData(uint64_t finalizingHeight, const BlockHeaderId *headerId, CodablecashBlockchain *blockchain) {
	this->finalizedHeight = finalizingHeight;
	saveStatus();
}

const BlockHead* ZoneStatusCache::getHead() const noexcept {
	return this->headBlockDetector->getHead();
}

const BlockHead* ZoneStatusCache::getSecondHead() const noexcept {
	return this->headBlockDetector->getSecondHead();
}

BigInteger ZoneStatusCache::calcTargetDifficulty(const BlockHeader* header, BlockHeaderStoreManager* headerManager
		, uint64_t powBlockTimeMills, uint16_t powHashrateBlocks, const SystemTimestamp* end) const {
	BigInteger N(0L);

	BlockHeaderId* lastHeaderId = nullptr;
	uint64_t height;

	SystemTimestamp tmStart, tmEnd;
	{
		//const BlockHead* head = getHead();
		//const BlockHeader* blockHeader = head->getHeadHeader();
		const PoWNonce* nonce = header->getPoWNonce();

		height = header->getHeight();

		PoWNonceResult* res = nonce->calcResult(header->getLastHeaderId(), header->getMerkleRoot(), header->getNonceGeneratedtimestamp()); __STP(res);
		BigInteger diff = res->getDifficulty();
		BigInteger ntimes = PoWNonce::calcDifficulty2Ntimes(diff);
		N.addSelf(ntimes);

		tmStart = *header->getTimestamp();
		tmEnd = *end;

		// last header id
		lastHeaderId = dynamic_cast<BlockHeaderId*>(header->getLastHeaderId()->copyData());
		height--;
	}

	int blocks = 1;
	for(int i = 1; i < powHashrateBlocks; ++i){
		BlockHeader* blockHeader = headerManager->getHeader(lastHeaderId, height); __STP(blockHeader);
		if(blockHeader == nullptr){
			break;
		}
		const PoWNonce* nonce = blockHeader->getPoWNonce();
		PoWNonceResult* res = nonce->calcResult(blockHeader->getLastHeaderId()
				, blockHeader->getMerkleRoot(), blockHeader->getNonceGeneratedtimestamp()); __STP(res);

		BigInteger diff = res->getDifficulty();
		BigInteger ntimes = PoWNonce::calcDifficulty2Ntimes(diff);
		N.addSelf(ntimes);

		tmStart = *blockHeader->getTimestamp();

		// last header id
		delete lastHeaderId;
		lastHeaderId = dynamic_cast<BlockHeaderId*>(blockHeader->getLastHeaderId()->copyData());
		height--;
	}
	__STP(lastHeaderId);

	// calc hash rate
	SystemTimestamp intervalSec = tmEnd - tmStart;
	if(intervalSec.isZero()){ // only genesis block
		SystemTimestamp tmp(1, 0);
		intervalSec = intervalSec + tmp;
	}

	BigInteger interval = intervalSec.toBigIntegerMills();

	BigInteger hashRate = N.multiply(BigInteger::THOUSAND).divide(interval);
	BigInteger diff = PoWNonce::calcNecessaryDifficulty(hashRate, powBlockTimeMills);

	// DEBUG:diff
	//assert(diff.compareTo(*PoWNonce::getMaxBigInt()) < 0);
	//UnicodeString hex = diff.toString(16);

	return diff;
}

UtxoData* ZoneStatusCache::findUtxo(const UtxoId *utxoId) const {
	return this->finalizedCache->findUtxo(utxoId);
}

BlockHeader* ZoneStatusCache::changeMiningTarget() {
	return this->headBlockDetector->changeMiningTarget();
}

void ZoneStatusCache::setScheduledBlock(const Block *block) {
	this->headBlockDetector->setScheduledBlock(block);
}

Block* ZoneStatusCache::fetchScheduledBlock() {
	return this->headBlockDetector->fetchScheduledBlock();
}

Block* ZoneStatusCache::getScheduledBlock() {
	return this->headBlockDetector->getScheduledBlock();
}

bool ZoneStatusCache::registerBlockHeader4Limit(const BlockHeader *header, const CodablecashSystemParam *param) {
	return this->voteManager->registerBlockHeader(header, param);
}

SystemTimestamp* ZoneStatusCache::getPosVoteLimit(uint64_t lastHeight) {
	return this->voteManager->getPosVoteLimit(lastHeight);
}

} /* namespace codablecash */
