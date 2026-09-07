/*
 * BlockHeaderStoreCache.cpp
 *
 *  Created on: 2023/02/16
 *      Author: iizuka
 */

#include "bc_blockstore_header/BlockHeaderStoreCache.h"
#include "bc_blockstore_header/BlockHeaderStoreCacheElement.h"

#include "base/StackRelease.h"

namespace codablecash {

BlockHeaderStoreCache::BlockHeaderStoreCache(const File* baseDir, int sectionLimit) {
	this->baseDir = baseDir;
	this->sectionLimit = sectionLimit;
	this->maxCache = 2;
	this->list = new RawLinkedList<BlockHeaderStoreCacheElement>();
}

BlockHeaderStoreCache::~BlockHeaderStoreCache() {
	RawLinkedList<BlockHeaderStoreCacheElement,RawCompare>::Iterator it = this->list->iterator();
	while(it.hasNext()){
		BlockHeaderStoreCacheElement* element = it.next();
		delete element;
	}

	delete this->list;

	this->baseDir = nullptr;
}

BlockHeaderStoreCacheElement* BlockHeaderStoreCache::getHeaderStoreCacheElement(uint64_t height, bool create) {
	uint64_t index = height / this->sectionLimit;

	BlockHeaderStoreCacheElement* cache = findCache(index);
	if(cache != nullptr){
		return cache;
	}

	if(this->list->size() == this->maxCache){
		RawLinkedList<BlockHeaderStoreCacheElement,RawCompare>::Element* lastElement =  this->list->getLastElement();

		BlockHeaderStoreCacheElement* lastCache = lastElement->data; __STP(lastCache);
		lastCache->close();

		this->list->remove(lastElement);
	}

	cache = new BlockHeaderStoreCacheElement(this->baseDir, index); __STP(cache);
	if(!cache->exist() && !create){
		return nullptr;
	}

	if(!cache->exist()){
		cache->create();
	}

	cache->open(); // open

	this->list->add(0, cache);

	return __STP_MV(cache);
}

BlockHeaderStoreCacheElement* BlockHeaderStoreCache::findCache(uint64_t index) noexcept {
	BlockHeaderStoreCacheElement* ret = nullptr;

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		BlockHeaderStoreCacheElement* cache = this->list->get(i);

		if(index == cache->getIndex()){
			this->list->moveElementToTop(i);
			ret = cache;
			break;
		}
	}

	return ret;
}

} /* namespace codablecash */
