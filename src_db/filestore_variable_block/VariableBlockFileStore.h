/*
 * VariableBlockFileStore.h
 *
 *  Created on: 2023/02/08
 *      Author: iizuka
 */

#ifndef FILESTORE_VARIABLE_BLOCK_VARIABLEBLOCKFILESTORE_H_
#define FILESTORE_VARIABLE_BLOCK_VARIABLEBLOCKFILESTORE_H_

#include "filestore/FileStore.h"
#include "filestore_block/IBlockFileStore.h"

#include "base/ArrayList.h"

namespace alinous {

class VariableBlockFileBody;
class VariableBlockHeader;
class VariableBlock;

class VariableBlockFileStore : public FileStore, public IBlockFileStore {
public:
	VariableBlockFileStore(const UnicodeString* dir, const UnicodeString* name, DiskCacheManager* cacheManager);
	virtual ~VariableBlockFileStore();

	virtual void createStore(bool del, uint64_t defaultSize, uint64_t blockSize) noexcept(false);
	void createStore(bool del, uint64_t defaultSize, uint64_t blockUnitSize, uint64_t extendBlocks) noexcept(false);

	virtual void open(bool sync) noexcept(false);
	virtual void close() noexcept;

	void extendFile();

	virtual IBlockHandle* alloc(uint64_t size);
	virtual IBlockHandle* realloc(uint64_t fpos, uint64_t size);
	virtual IBlockHandle* get(uint64_t fpos);
	ArrayList<VariableBlock>* getBlockList(uint64_t fpos);
	virtual void sync(bool fsync);

	VariableBlockFileBody* getBody() const noexcept {
		return this->body;
	}
	VariableBlockHeader* getHeader() const noexcept {
		return this->header;
	}

private:
	IBlockHandle* blocksToHandle(ArrayList<VariableBlock>* list);
	void internalClear() noexcept;

private:
	VariableBlockFileBody* body;
	VariableBlockHeader* header;
};

} /* namespace alinous */

#endif /* FILESTORE_VARIABLE_BLOCK_VARIABLEBLOCKFILESTORE_H_ */
