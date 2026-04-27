/*
 * CodablecashBlockchain.cpp
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#include "bc_blockstore/CodablecashBlockchain.h"
#include "bc_blockstore/ZoneStore.h"
#include "bc_blockstore/BlockchainStoreException.h"
#include "bc_blockstore/IBlockchainEventListner.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_blockstore_body/BlockBodyStoreManager.h"

#include "bc_base_conf_store/StatusStore.h"

#include "base_io/File.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "base_thread/ConcurrentGate.h"
#include "base_thread/SysMutex.h"

#include "bc/SoftwareVersion.h"

#include "bc_block/Block.h"
#include "bc_block/BlockHeader.h"

#include "bc_block_body/BlockBody.h"

#include "bc_processor/CentralProcessor.h"

#include "base_thread/StackUnlocker.h"

#include "bc_block/BlockMerkleRoot.h"
namespace codablecash {

const UnicodeString CodablecashBlockchain::CONFIG_BIN_FILE(L"config.bin");

const UnicodeString CodablecashBlockchain::KEY_SECTION_LIMIT(L"sectionLimit");
const UnicodeString CodablecashBlockchain::KEY_ZONE_SELF(L"zoneSelf");
const UnicodeString CodablecashBlockchain::KEY_NUM_ZONES(L"numZones");

const UnicodeString CodablecashBlockchain::KEY_BLOCK_VERSION_MAJOR(L"version_major");
const UnicodeString CodablecashBlockchain::KEY_BLOCK_VERSION_MINOR(L"version_minor");
const UnicodeString CodablecashBlockchain::KEY_BLOCK_VERSION_PATCH(L"version_patch");

CodablecashBlockchain::CodablecashBlockchain(const File *baseDir, uint16_t zoneSelf, uint16_t numZones) {
	this->baseDir = new File(*baseDir);
	this->zonesStore = new ArrayList<ZoneStore>();
	this->zoneSelf = zoneSelf;
	this->numZones = numZones;

	this->sectionLimit = DEFAULT_SECTION_LIMIT;

	this->configStore = nullptr;
	this->version = new SoftwareVersion(0, 1, 0);

	this->rwLock = new ConcurrentGate();
	this->processor = nullptr;
	this->listnerLock = new SysMutex();
}

CodablecashBlockchain::CodablecashBlockchain(const File* baseDir) {
	this->baseDir = new File(*baseDir);
	this->zonesStore = new ArrayList<ZoneStore>();
	this->zoneSelf = 0;
	this->numZones = 0;
	this->sectionLimit = 0;
	this->configStore = nullptr;
	this->version = new SoftwareVersion(0, 1, 0);

	this->rwLock = new ConcurrentGate();
	this->processor = nullptr;
	this->listnerLock = new SysMutex();
}

CodablecashBlockchain::~CodablecashBlockchain() {
	close();

	delete this->baseDir;
	delete this->version;
	delete this->rwLock;
	delete this->listnerLock;
}

void CodablecashBlockchain::open() {
	this->configStore = new StatusStore(this->baseDir, &CodablecashBlockchain::CONFIG_BIN_FILE);
	loadCondig();

	// init zone store instances
	{
		int maxLoop = this->numZones;
		for(int i = 0; i != maxLoop; ++i){
			uint16_t zone = i;
			initZone(zone);
		}
	}

	int maxLoop = this->zonesStore->size();
	for(int i = 0; i != maxLoop; ++i){
		ZoneStore* store = this->zonesStore->get(i);
		store->open();
	}
}

void CodablecashBlockchain::close() noexcept {
	if(this->zonesStore != nullptr){
		int maxLoop = this->zonesStore->size();
		for(int i = 0; i != maxLoop; ++i){
			ZoneStore* store = this->zonesStore->get(i);
			store->close();
		}

		this->zonesStore->deleteElements();
		delete this->zonesStore;
		this->zonesStore = nullptr;
	}

	delete this->configStore;
	this->configStore = nullptr;
}

void CodablecashBlockchain::createBlankBlockchain(const File *baseDir, uint16_t zoneSelf, uint16_t numZones) {
	if(baseDir->exists()){
		throw new BlockchainStoreException(L"Blockchain already exists.", __FILE__, __LINE__);
	}
	baseDir->mkdirs();

	CodablecashBlockchain* chain = new CodablecashBlockchain(baseDir, zoneSelf, numZones); __STP(chain);
	chain->initBlankchain();
}

void CodablecashBlockchain::initBlankchain() {
	this->configStore = new StatusStore(this->baseDir, &CodablecashBlockchain::CONFIG_BIN_FILE);

	saveCondig();
	initZonesStore();
}

void CodablecashBlockchain::initZonesStore() {
	int maxLoop = this->numZones;
	for(int i = 0; i != maxLoop; ++i){
		uint16_t zone = i;
		initZone(zone);
	}
}

void CodablecashBlockchain::initZone(uint16_t zone) {
	char tmp[6];
	Mem::memset(tmp, 0, 6);

	::sprintf(tmp, "%03d", zone);

	UnicodeString zoneDirSeg((const char*)tmp);
	File* zoneDir = this->baseDir->get(&zoneDirSeg); __STP(zoneDir);

	bool headerOnly = (zone != this->zoneSelf);

	ZoneStore* store = new ZoneStore(zone, zoneDir, headerOnly, this->sectionLimit);
	this->zonesStore->addElement(store);
}

void CodablecashBlockchain::saveCondig() {
	this->configStore->addShortValue(&KEY_SECTION_LIMIT, this->sectionLimit);
	this->configStore->addShortValue(&KEY_ZONE_SELF, this->zoneSelf);
	this->configStore->addShortValue(&KEY_NUM_ZONES, this->numZones);

	this->configStore->addShortValue(&KEY_BLOCK_VERSION_MAJOR, this->version->getMajor());
	this->configStore->addShortValue(&KEY_BLOCK_VERSION_MINOR, this->version->getMinor());
	this->configStore->addShortValue(&KEY_BLOCK_VERSION_PATCH, this->version->getPatch());
}

void CodablecashBlockchain::addListner(IBlockchainEventListner *listner) noexcept {
	StackUnlocker lock(this->listnerLock, __FILE__, __LINE__);

	this->listners.addElement(listner);
}

void CodablecashBlockchain::fireBlockAdded(MemPoolTransaction* memTrx, const Block *block) {
	StackUnlocker lock(this->listnerLock, __FILE__, __LINE__);

	int maxLoop = this->listners.size();
	for(int i = 0; i != maxLoop; ++i){
		IBlockchainEventListner* listner = this->listners.get(i);
		listner->onBlockAdded(memTrx, block, this);
	}
}

void CodablecashBlockchain::firePostBlockAdded(const Block *block) {
	StackUnlocker lock(this->listnerLock, __FILE__, __LINE__);

	int maxLoop = this->listners.size();
	for(int i = 0; i != maxLoop; ++i){
		IBlockchainEventListner* listner = this->listners.get(i);

		listner->postBlockAdded(block, this);
	}
}

void CodablecashBlockchain::fireBlockHeaderAdded(MemPoolTransaction *memTrx, const BlockHeader *header) {
	StackUnlocker lock(this->listnerLock, __FILE__, __LINE__);

	int maxLoop = this->listners.size();
	for(int i = 0; i != maxLoop; ++i){
		IBlockchainEventListner* listner = this->listners.get(i);
		listner->onBlockHeaderAdded(memTrx, header, this);
	}
}

void CodablecashBlockchain::loadCondig() {
	this->configStore->load();

	this->sectionLimit = this->configStore->getShortValue(&KEY_SECTION_LIMIT);
	this->zoneSelf = this->configStore->getShortValue(&KEY_ZONE_SELF);
	this->numZones = this->configStore->getShortValue(&KEY_NUM_ZONES);

	int major = this->configStore->getShortValue(&KEY_BLOCK_VERSION_MAJOR);
	int minor = this->configStore->getShortValue(&KEY_BLOCK_VERSION_MINOR);
	int patch = this->configStore->getShortValue(&KEY_BLOCK_VERSION_PATCH);

	delete this->version;
	this->version = new SoftwareVersion(major, minor, patch);
}

void CodablecashBlockchain::addBlock(MemPoolTransaction* memTrx, const Block *block) {
	{
		StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

		uint16_t zone = block->getZone();

		ZoneStore* store = this->zonesStore->get(zone);
		store->addBlock(block);
	}

	fireBlockAdded(memTrx, block);
}

void CodablecashBlockchain::addHeader(MemPoolTransaction *memTrx, const BlockHeader *header) {
	{
		StackWriteLock __lock(this->rwLock, __FILE__, __LINE__);

		uint16_t zone = header->getZone();
		ZoneStore* store = this->zonesStore->get(zone);

		store->addBlockHeader(header);
	}

	fireBlockHeaderAdded(memTrx, header);
}

BlockHeaderStoreManager* CodablecashBlockchain::getHeaderManager(uint16_t zone) const noexcept {
	assert(zone < this->zonesStore->size());

	ZoneStore* store = this->zonesStore->get(zone);
	BlockHeaderStoreManager* ret = store->getBlockHeaderStoreManager();
	return ret;
}

BlockBodyStoreManager* CodablecashBlockchain::getBlockBodyStoreManager(uint16_t zone) const noexcept {
	assert(zone < this->zonesStore->size());

	ZoneStore* store = this->zonesStore->get(zone);

	return store->getBlockBodyStoreManager();
}

void CodablecashBlockchain::cleanOnFinalize(uint16_t zone,	uint64_t finalizingHeight, const BlockHeaderId *headerId, uint16_t lastFinalizedHeight) {
	ZoneStore* store = this->zonesStore->get(zone);

	store->cleanOnFinalize(finalizingHeight, headerId, lastFinalizedHeight);
}

ArrayList<Block>* CodablecashBlockchain::getBlocksHeightAt(uint16_t zone, uint64_t height) const {
	ZoneStore* store = this->zonesStore->get(zone);
	BlockHeaderStoreManager* headerManager = store->getBlockHeaderStoreManager();
	BlockBodyStoreManager* bodyManaer = store->getBlockBodyStoreManager();

	ArrayList<BlockHeader>* headers = headerManager->getBlocksAtHeight(height); __STP(headers);
	if(headers == nullptr){
		return nullptr;
	}
	headers->setDeleteOnExit();

	ArrayList<Block>* list = new ArrayList<Block>();
	{
		int maxLoop = headers->size();
		for(int i = 0; i != maxLoop; ++i){
			BlockHeader* header = headers->get(i);
			const BlockMerkleRoot *root = header->getMerkleRoot();

			BlockBody* body = bodyManaer->getBlockBody(root, height); __STP(body);

#ifdef __DEBUG__
			{
				const BlockMerkleRoot* rootheader = header->getMerkleRoot();
				const BlockMerkleRoot* rootbody = body->getMerkleRoot();

				UnicodeString* str1 = rootheader->toString(); __STP(str1);
				UnicodeString* str2 = rootbody->toString(); __STP(str2);

				bool bl = rootheader->equals(rootbody);
				assert(bl == true);
			}
#endif
			Block* block = new Block(dynamic_cast<BlockHeader*>(header->copyData()), __STP_MV(body));
			list->addElement(block);
		}
	}

	return list;
}

} /* namespace codablecash */
