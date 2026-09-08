/*
 * HeaderStoreCacheElement.cpp
 *
 *  Created on: 2023/02/16
 *      Author: iizuka
 */

#include "bc_blockstore_header/BlockHeaderStoreCacheElement.h"
#include "bc_blockstore_header/BlockHeaderHeightIndex.h"
#include "bc_blockstore_header/BlockHeaderStore.h"
#include "bc_blockstore_header/BlockHeaderList.h"
#include "bc_blockstore_header/IHeaderRemovalNotifier.h"

#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"

#include "base_io/File.h"

#include "random_access_file/DiskCacheManager.h"

#include "osenv/funcs.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"


namespace codablecash {

BlockHeaderStoreCacheElement::BlockHeaderStoreCacheElement(const File* baseDir, uint64_t index) {
	this->baseDir = new File(*baseDir);
	this->index = index;

	this->cacheManager = new DiskCacheManager(Os::getSystemPageSize() * 4 * 4);
	this->store = nullptr;
	this->heightIndex = nullptr;
}

BlockHeaderStoreCacheElement::~BlockHeaderStoreCacheElement() {
	close();

	delete this->baseDir;
	delete this->cacheManager;
}

void BlockHeaderStoreCacheElement::open() {
	this->store = new BlockHeaderStore(this->index, this->baseDir, this->cacheManager);
	this->heightIndex = new BlockHeaderHeightIndex(this->index, this->baseDir, this->cacheManager);

	this->store->open();
	this->heightIndex->open();
}

bool BlockHeaderStoreCacheElement::exist() const noexcept {
	BlockHeaderStore s(this->index, this->baseDir, this->cacheManager);
	BlockHeaderHeightIndex h(this->index, this->baseDir, this->cacheManager);

	bool storeexist = s.exists();
	bool indexexists = h.exists();

	return storeexist && indexexists;
}

void BlockHeaderStoreCacheElement::create() {
	BlockHeaderStore s(this->index, this->baseDir, this->cacheManager);
	BlockHeaderHeightIndex h(this->index, this->baseDir, this->cacheManager);

	s.create();
	h.create();
}

void BlockHeaderStoreCacheElement::close() {
	if(this->store != nullptr){
		this->store->close();
		delete this->store, this->store = nullptr;
	}
	if(this->heightIndex != nullptr){
		this->heightIndex->close();
		delete this->heightIndex, this->heightIndex = nullptr;
	}
}

void BlockHeaderStoreCacheElement::addHeader(const BlockHeader *header) {
	uint64_t fpos = this->store->storeHeader(header);

	uint64_t height = header->getHeight();
	this->heightIndex->addHeader(height, fpos);
}

ArrayList<BlockHeader>* BlockHeaderStoreCacheElement::getHeadersAtHeight(uint64_t height) {
	BlockHeaderList* list = this->heightIndex->getHeadersAtHeight(height); __STP(list);

	if(list == nullptr){
		return nullptr;
	}

	ArrayList<BlockHeader>* headers = new ArrayList<BlockHeader>();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		uint64_t fpos = list->get(i);

		BlockHeader* header = this->store->loadHeader(fpos);
		headers->addElement(header);
	}

	return headers;
}

ArrayList<BlockHeader>* BlockHeaderStoreCacheElement::getChildrenOf(const BlockHeaderId *headerId, uint64_t height) {
	uint64_t childHeight = height + 1;

	BlockHeaderList* list = this->heightIndex->getHeadersAtHeight(childHeight); __STP(list);

	if(list == nullptr){
		return new ArrayList<BlockHeader>();
	}

	ArrayList<BlockHeader>* headers = new ArrayList<BlockHeader>();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		uint64_t fpos = list->get(i);

		BlockHeader* header = this->store->loadHeader(fpos); __STP(header);
		const BlockHeaderId* lastHeaderId = header->getLastHeaderId();

		if(lastHeaderId->equals(headerId)){
			headers->addElement(__STP_MV(header));
		}
	}

	return headers;
}

void BlockHeaderStoreCacheElement::removeBlock(const BlockHeaderId *hash, uint64_t height) {
	BlockHeaderList* list = this->heightIndex->getHeadersAtHeight(height); __STP(list);
	if(list == nullptr){
		return;
	}

	uint64_t fpos2remove = -1;

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		uint64_t fpos = list->get(i);

		BlockHeader* header = this->store->loadHeader(fpos); __STP(header);
		const BlockHeaderId* id = header->getId();

		if(id->equals(hash)){
			fpos2remove = fpos;
			break;
		}
	}

	assert(fpos2remove > 0);

	this->heightIndex->removeIndex(height, fpos2remove);
	this->store->removeHeader(fpos2remove);
}

void BlockHeaderStoreCacheElement::finalize(uint64_t height, const BlockHeaderId *headerId, IHeaderRemovalNotifier* notifier) {
	BlockHeaderList* list = this->heightIndex->getHeadersAtHeight(height); __STP(list);

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		uint64_t fpos = list->get(i);

		BlockHeader* header = this->store->loadHeader(fpos); __STP(header);
		const BlockHeaderId* id = header->getId();
		if(headerId->equals(id)){
			continue;
		}

		// remove header
		this->heightIndex->removeIndex(height, fpos);
		this->store->removeHeader(fpos);

		// call body deleter
		if(notifier != nullptr){
			notifier->onRemovedHeader(header);
		}
	}
}

} /* namespace codablecash */
