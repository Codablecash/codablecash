/*
 * BtreeStorage.cpp
 *
 *  Created on: 2018/12/18
 *      Author: iizuka
 */

#include "btree/BtreeStorage.h"
#include "btree/BtreeHeaderBlock.h"
#include "btree/TreeNode.h"
#include "btree/DataNode.h"
#include "btree/BtreeConfig.h"
#include "btree/NodeHandle.h"
#include "btree/AbstractBtreeDataFactory.h"

#include "btree_cache/NodeCache.h"

#include "btreekey/InfinityKey.h"

#include "base_io/File.h"
#include "base_io/ReverseByteBuffer.h"

#include "filestore_block/IBlockHandle.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_thread/StackUnlocker.h"

#include "filestore_block/IBlockFileStore.h"

#include "bc_base/BinaryUtils.h"

#include "filestore_variable_block/VariableBlockFileStore.h"
namespace alinous {

BtreeStorage::BtreeStorage(File* folder, UnicodeString* name, BtreeKeyFactory* factory, AbstractBtreeDataFactory* dfactory) {
	this->name = name;
	this->folder = folder;
	this->factory = factory;
	this->store = nullptr;
	this->cache = nullptr;
	this->rootFpos = 0;
	this->dfactory = dfactory;
}

BtreeStorage::~BtreeStorage() {
	this->name = nullptr;
	this->folder = nullptr;
	if(this->store != nullptr){
		delete this->store, this->store = nullptr;
	}
	if(this->cache != nullptr){
		delete this->cache, this->cache = nullptr;
	}
}

bool BtreeStorage::exists() const noexcept {
	UnicodeString* folderstr = this->folder->getAbsolutePath();
	StackRelease<UnicodeString> __st_folderstr(folderstr);

	//BlockFileStore* blockstore = new BlockFileStore(folderstr, this->name, nullptr);
	//StackRelease<BlockFileStore> __st_blockstore(blockstore);

	VariableBlockFileStore* blockstore = new VariableBlockFileStore(folderstr, this->name, nullptr);
	StackRelease<VariableBlockFileStore> __st_blockstore(blockstore);

	return blockstore->exists();
}

void BtreeStorage::create(DiskCacheManager* cacheManager, const BtreeConfig* config) {
	UnicodeString* folderstr = this->folder->getAbsolutePath();
	StackRelease<UnicodeString> __st_folderstr(folderstr);

	IBlockFileStore* blockstore = new VariableBlockFileStore(folderstr, this->name, cacheManager);
	StackRelease<IBlockFileStore> __st_blockstore(blockstore);

	blockstore->createStore(true, config->defaultSize, config->blockSize);

	blockstore->open(false);

	uint64_t rootFpos;
	{
		// pre alloc
		IBlockHandle* handle = blockstore->alloc(1);
		delete handle;

		handle = blockstore->alloc(1);

		rootFpos = handle->getFpos();
		delete handle;
	}

	// root node
	{
		IBlockHandle* handle = blockstore->get(rootFpos);
		StackRelease<IBlockHandle> __st_handle(handle);

		InfinityKey* infinityKey = new InfinityKey();
		TreeNode rootNode(true, config->nodeNumber, infinityKey, true);
		rootNode.setFpos(rootFpos);

		int cap = rootNode.binarySize();
		ByteBuffer* buff = ReverseByteBuffer::allocateWithEndian(cap, true);
		StackRelease<ByteBuffer> __st_buff(buff);

		rootNode.toBinary(buff);

		handle->write((const char*)buff->array(), cap);

		assert(handle->getFpos() == rootFpos);
	}

	{
		// first header
		BtreeHeaderBlock* header = makeHeader(config, rootFpos);
		StackRelease<BtreeHeaderBlock> __st_header(header);


		int headerSize = header->binarySize();
		ByteBuffer* buff = ByteBuffer::allocateWithEndian(headerSize, true);
		StackRelease<ByteBuffer> __st_buff(buff);

		header->toBinary(buff);
		buff->position(0);

		IBlockHandle* handle = blockstore->get(0);
		StackRelease<IBlockHandle> __st_handle(handle);

		const char* ar = (const char*)buff->array();
		handle->write(ar, headerSize);

		assert(handle->getFpos() == 0);
	}

	blockstore->sync(false);

	blockstore->close();
}

BtreeHeaderBlock* BtreeStorage::makeHeader(const BtreeConfig* config, uint64_t rootFpos) {
	BtreeHeaderBlock* header = new BtreeHeaderBlock();
	header->setConfig(config);
	header->setRootFpos(rootFpos);

	return header;
}

void BtreeStorage::updateRootFpos(uint64_t rootFpos) {
	this->rootFpos = rootFpos;

	IBlockHandle* handle = this->store->get(0);
	StackRelease<IBlockHandle> __st_handle(handle);

	ByteBuffer* in = handle->getBuffer();
	in->position(0);

	BtreeHeaderBlock* btreeHeader = BtreeHeaderBlock::fromBinary(in);
	StackRelease<BtreeHeaderBlock> __st_btreeHeader(btreeHeader);
	btreeHeader->setRootFpos(rootFpos);

	int binarySize = btreeHeader->binarySize();
	ByteBuffer* buff = ReverseByteBuffer::allocateWithEndian(binarySize, true);
	StackRelease<ByteBuffer> __st_buff(buff);

	btreeHeader->toBinary(buff);

	handle->write((const char*)buff->array(), binarySize);
}

void BtreeStorage::open(int numDataBuffer, int numNodeBuffer, DiskCacheManager* cacheManager) {
	UnicodeString* folderstr = this->folder->getAbsolutePath();
	StackRelease<UnicodeString> __st_folderstr(folderstr);

	this->store = new VariableBlockFileStore(folderstr, this->name, cacheManager);
	this->store->open(false);

	this->cache = new NodeCache(numDataBuffer, numNodeBuffer);
}

BtreeHeaderBlock* BtreeStorage::loadHeader() {
	// load 0 fpos
	IBlockHandle* handle = this->store->get(0);
	StackRelease<IBlockHandle> __st_handle(handle);

	ByteBuffer* buff = handle->getBuffer();
	buff->position(0);

	BtreeHeaderBlock* header = BtreeHeaderBlock::fromBinary(buff);
	return header;
}

void BtreeStorage::close() noexcept {
	this->store->close();
	this->cache->clear();
}

void BtreeStorage::sync(bool syncDisk) {
	this->store->sync(syncDisk);
}

NodeHandle* BtreeStorage::loadRoot() {
	return loadNode(this->rootFpos);
}

void BtreeStorage::setRootFpos(uint64_t rootFpos){
	this->rootFpos = rootFpos;
}

NodeHandle* BtreeStorage::loadNode(uint64_t fpos) {
	StackUnlocker __lock(&this->lock, __FILE__, __LINE__);

	NodeCacheRef* ref = this->cache->get(fpos);
	if(ref != nullptr){
		return new NodeHandle(ref);
	}

	IBlockHandle* handle = this->store->get(fpos);
	StackRelease<IBlockHandle> __st_handle(handle);

	ByteBuffer* buff = handle->getBuffer();
	buff->position(0);

	AbstractTreeNode* node = BtreeStorage::makeNodeFromBinary(buff, this->factory);
	__ASSERT_POS(buff);
	assert(node->getFpos() == fpos);

	this->cache->add(node);
	ref = this->cache->get(fpos);

	return new NodeHandle(ref);
}

IBlockObject* BtreeStorage::loadData(uint64_t fpos) {
	StackUnlocker __lock(&this->lock, __FILE__, __LINE__);

	IBlockHandle* handle = this->store->get(fpos);
	StackRelease<IBlockHandle> __st_handle(handle);

	ByteBuffer* buff = handle->getBuffer();
	buff->position(0);

	return this->dfactory->makeDataFromBinary(buff);
}

AbstractTreeNode* BtreeStorage::makeNodeFromBinary(ByteBuffer* buff, BtreeKeyFactory* factory) {
	char nodeType = buff->get();

	if(nodeType == AbstractTreeNode::NODE){
		return TreeNode::fromBinary(buff, factory);
	}

	assert(nodeType == AbstractTreeNode::DATA);
	return DataNode::fromBinary(buff, factory);
}

void BtreeStorage::remove(uint64_t fpos) {
	StackUnlocker __lock(&this->lock, __FILE__, __LINE__);

	// clear cache
	NodeCacheRef* ref = this->cache->get(fpos);
	if(ref != nullptr){
		// delete cache object
		this->cache->remove(ref);
	}

	IBlockHandle* handle = this->store->get(fpos);
	StackRelease<IBlockHandle> __st_handle(handle);
	handle->removeBlocks();
}


uint64_t BtreeStorage::storeData(const IBlockObject* data) {
	int size = data->binarySize();
	IBlockHandle* handle = this->store->alloc(size);
	StackRelease<IBlockHandle> __st_handle(handle);

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(size, true);
	StackRelease<ByteBuffer> __st_buff(buff);

	data->toBinary(buff);
	__ASSERT_POS(buff);

	const char* ptr = (const char*)buff->array();
	handle->write(ptr, size);

	return handle->getFpos();
}

uint64_t BtreeStorage::storeData(const IBlockObject* data, uint64_t fpos) {
	IBlockHandle* handle = this->store->get(fpos);
	StackRelease<IBlockHandle> __st_handle(handle);

	int size = data->binarySize();
	ByteBuffer* buff = ReverseByteBuffer::allocateWithEndian(size, true);
	StackRelease<ByteBuffer> __st_buff(buff);

	data->toBinary(buff);
	__ASSERT_POS(buff);

	const char* ptr = (const char*)buff->array();
	handle->write(ptr, size);

	return handle->getFpos();
}

void BtreeStorage::removeData(uint64_t dataFpos) {
	IBlockHandle* handle = this->store->get(dataFpos);
	StackRelease<IBlockHandle> __st_handle(handle);

	handle->removeBlocks();
}

uint64_t BtreeStorage::storeNode(AbstractTreeNode* node) {
	int size = node->binarySize();
	IBlockHandle* handle = this->store->alloc(size);
	StackRelease<IBlockHandle> __st_handle(handle);

	ByteBuffer* buff = ReverseByteBuffer::allocateWithEndian(size, true);
	StackRelease<ByteBuffer> __st_buff(buff);


	uint64_t fpos = handle->getFpos();
	node->setFpos(fpos);
	node->toBinary(buff);

	const char* ptr = (const char*)buff->array();
	handle->write(ptr, size);

	assert(fpos == handle->getFpos());

	return handle->getFpos();
}

void BtreeStorage::updateNode(AbstractTreeNode* node) {
	int size = node->binarySize();

	uint64_t fpos = node->getFpos();
	IBlockHandle* handle = this->store->get(fpos);
	StackRelease<IBlockHandle> __st_handle(handle);

	ByteBuffer* buff = ReverseByteBuffer::allocateWithEndian(size, true);
	StackRelease<ByteBuffer> __st_buff(buff);

	node->toBinary(buff);
	__ASSERT_POS(buff);

	const char* ptr = (const char*)buff->array();
	handle->write(ptr, size);

#ifdef __DEBUG__
	{
		uint64_t cuurentFpos = handle->getFpos();
		assert(fpos == cuurentFpos);
	}
#endif
}

const AbstractBtreeDataFactory* BtreeStorage::getDataFactory() const noexcept {
	return this->dfactory;
}

} /* namespace alinous */
