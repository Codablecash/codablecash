/*
 * HeaderStoreCacheElement.h
 *
 *  Created on: 2023/02/16
 *      Author: iizuka
 */

#ifndef BC_BLOCKSTORE_HEADER_BLOCKHEADERSTORECACHEELEMENT_H_
#define BC_BLOCKSTORE_HEADER_BLOCKHEADERSTORECACHEELEMENT_H_

namespace alinous {
class File;
class DiskCacheManager;
}
using namespace alinous;

#include <cstdint>

#include "base/ArrayList.h"

namespace codablecash {

class BlockHeaderStore;
class BlockHeaderHeightIndex;
class BlockHeader;
class BlockHeaderId;
class IHeaderRemovalNotifier;

class BlockHeaderStoreCacheElement {
public:
	BlockHeaderStoreCacheElement(const File* baseDir, uint64_t index);
	virtual ~BlockHeaderStoreCacheElement();

	void open();

	bool exist() const noexcept;
	void create();

	void close();

	void addHeader(const BlockHeader* header);
	ArrayList<BlockHeader>* getHeadersAtHeight(uint64_t height);
	ArrayList<BlockHeader>* getChildrenOf(const BlockHeaderId *headerId, uint64_t height);

	void removeBlock(const BlockHeaderId* hash, uint64_t height);

	void finalize(uint64_t height,	const BlockHeaderId *headerId, IHeaderRemovalNotifier* notifier);

	uint64_t getIndex() const noexcept {
		return index;
	}

private:
	uint64_t index;
	File* baseDir;
	DiskCacheManager* cacheManager;

	BlockHeaderStore* store;
	BlockHeaderHeightIndex* heightIndex;
};

} /* namespace codablecash */

#endif /* BC_BLOCKSTORE_HEADER_BLOCKHEADERSTORECACHEELEMENT_H_ */
