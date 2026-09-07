/*
 * VariableBlockHeader.cpp
 *
 *  Created on: 2023/02/08
 *      Author: iizuka
 */

#include "filestore_variable_block/VariableBlockHeader.h"
#include "filestore_variable_block/VariableBlock.h"

#include "random_access_file/RandomAccessFile.h"

#include "filestore/LongRange.h"
#include "filestore/LongRangeList.h"
#include "filestore/LongRangeIterator.h"

#include "filestore_block/exceptions.h"

#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"


namespace alinous {

VariableBlockHeader::VariableBlockHeader(RandomAccessFile* file) {
	this->file = file;
	this->availableArea = 0;
	this->blockUnitSize = 0;
	this->numBlocks = 0;
	this->extendBlocks = 0;
}

VariableBlockHeader::~VariableBlockHeader() {
	this->file = nullptr;
	delete this->availableArea;
}

void VariableBlockHeader::createStore(bool del, uint64_t defaultSize,	uint64_t blockUnitSize, uint64_t extendBlocks) {
	assert(defaultSize % blockUnitSize == 0);

	this->blockUnitSize = blockUnitSize;
	this->extendBlocks = extendBlocks;
	this->numBlocks = defaultSize / blockUnitSize;
	this->availableArea = new LongRangeList();
	this->availableArea->addRange(0, (defaultSize / blockUnitSize) - 1);

	sync(false);
}

uint64_t VariableBlockHeader::extend() {
	uint64_t min = this->numBlocks;
	uint64_t max = min + this->extendBlocks - 1;

	this->availableArea->addRange(min, max);

	this->numBlocks += this->extendBlocks;

	return this->numBlocks;
}

void VariableBlockHeader::sync(bool fileSync) {
	sync2File();
	this->file->sync(fileSync);
}

void VariableBlockHeader::sync2File() {
	uint32_t headSize = sizeof(uint64_t) * 4;
	int contentSize = this->availableArea->binarySize();
	int binSize = headSize + contentSize;

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(binSize, true);
	StackRelease<ByteBuffer> _st_buff(buff);

	buff->putLong(binSize);
	buff->putLong(this->blockUnitSize);
	buff->putLong(this->extendBlocks);
	buff->putLong(this->numBlocks);

	this->availableArea->toBinary(buff);
	buff->position(0);


	// sync with file
	this->file->setLength(binSize + headSize);

	// file size
	ByteBuffer* buffSizeHeader = ByteBuffer::allocateWithEndian(headSize, true);
	StackRelease<ByteBuffer> _st_buffSizeHeader(buffSizeHeader);
	buffSizeHeader->putLong(binSize);
	buffSizeHeader->putLong(this->blockUnitSize);
	buffSizeHeader->putLong(this->extendBlocks);
	buffSizeHeader->putLong(this->numBlocks);
	buffSizeHeader->position(0);

	uint64_t fpos = 0;
	const char* binary = (const char*)buffSizeHeader->array();
	fpos += this->file->write(fpos, binary,headSize);

	// content
	binary = ((const char*)buff->array()) + headSize;
	int cnt = this->file->write(fpos, binary, contentSize);
	assert(cnt == contentSize);
}

void VariableBlockHeader::loadFromFile() {
	uint64_t fpos = 0;
	uint32_t headSize = sizeof(uint64_t) * 4;
	char sizeHeaderBinary[headSize]{};
	fpos += this->file->read(fpos, sizeHeaderBinary, headSize);

	ByteBuffer* buffSizeHeader = ByteBuffer::allocateWithEndian(headSize, true);
	StackRelease<ByteBuffer> _st_buffSizeHeader(buffSizeHeader);

	buffSizeHeader->put((const uint8_t*)sizeHeaderBinary, headSize);

	buffSizeHeader->position(0);
	int64_t loadSize = buffSizeHeader->getLong();
	this->blockUnitSize = buffSizeHeader->getLong();
	this->extendBlocks = buffSizeHeader->getLong();
	this->numBlocks = buffSizeHeader->getLong();

	int areaSize = loadSize - headSize;
	if(areaSize < 4){
		throw new BlockFileStorageException(L"File header format is broken", __FILE__, __LINE__);
	}

	char* usedAreaBinary = new char[areaSize]{};
	StackArrayRelease<char> st_usedAreaBinary(usedAreaBinary);
	fpos += this->file->read(fpos, usedAreaBinary, areaSize);

	ByteBuffer* rangeBinary = ByteBuffer::allocateWithEndian(areaSize, true);
	StackRelease<ByteBuffer> _st_rangeBinary(rangeBinary);

	rangeBinary->put((unsigned char*)usedAreaBinary, areaSize);
	rangeBinary->position(0);
	this->availableArea = LongRangeList::fromBinary(rangeBinary);
}

VariableBlock* VariableBlockHeader::reallocFirstMaxFragment(uint64_t firstBlockPos, uint64_t size) {
	LongRange* range = nullptr;

	int maxLoop = this->availableArea->size();
	for(int i = 0; i != maxLoop; ++i){
		const LongRange* r = this->availableArea->get(i);
		uint64_t min = r->getMin();
		uint64_t max = r->getMax();

		if(min <= firstBlockPos && firstBlockPos <= max){
			range = new LongRange(r);
			range->setMin(firstBlockPos);
			break;
		}
	}
	__STP(range);

	assert(range != nullptr);

	return allocMaxFragment(range, size);
}

VariableBlock* VariableBlockHeader::allocMaxFragment(uint64_t size) {
	const LongRange* range = this->availableArea->get(0);

	return allocMaxFragment(range, size);
}

VariableBlock* VariableBlockHeader::allocMaxFragment(const LongRange *range, uint64_t size) {
	uint64_t maxAvailable = availableWithRange(range);
	if(size > maxAvailable){
		return allocateAll(range);
	}

	int numBlock = getNumAllocationBlocks(size);

	uint64_t fpos = range->getMin() * this->blockUnitSize;
	VariableBlock* block = new VariableBlock(this->blockUnitSize * numBlock, fpos, /*used*/ size, /*nextfpos*/0, nullptr);

	uint64_t min = range->getMin();
	LongRange allocatedRange(min, min + numBlock - 1);

	this->availableArea->removeRange(&allocatedRange);

	return block;
}

int VariableBlockHeader::getNumAllocationBlocks(uint64_t size) {
	int64_t remain = size - (this->blockUnitSize - VariableBlock::HEADER_SIZE);
	if(remain <= 0){
		return 1;
	}

	// secondary
	int num = remain / this->blockUnitSize;
	int mod = remain % this->blockUnitSize;

	return (mod == 0) ? num + 1 : num + 2; // add first block
}

VariableBlock* VariableBlockHeader::allocateAll(const LongRange *range) {
	int blockBytes = range->width() * this->blockUnitSize;
	int used = blockBytes - VariableBlock::HEADER_SIZE;

	uint64_t fpos = range->getMin() * this->blockUnitSize;
	VariableBlock* block = new VariableBlock(blockBytes, fpos, used, /*nextfpos*/0, nullptr);

	this->availableArea->removeRange(range);

	return block;
}

uint64_t VariableBlockHeader::availableWithRange(const LongRange *range) const noexcept {
	uint64_t numBlocks = range->width();

	return (numBlocks * this->blockUnitSize) - VariableBlock::HEADER_SIZE;
}

bool VariableBlockHeader::isEmpty() const noexcept {
	return this->availableArea->isEmpty();
}

void VariableBlockHeader::freeFragment(const LongRange *range) {
	this->availableArea->addRange(range);
}

uint64_t VariableBlockHeader::availableCapacity() const noexcept {
	uint64_t ret = 0;

	int maxLoop = this->availableArea->size();
	for(int i = 0; i != maxLoop; ++i){
		const LongRange* range = this->availableArea->get(i);
		uint64_t width = range->width();

		ret += (width * this->blockUnitSize) - VariableBlock::HEADER_SIZE;
	}

	return ret;
}

} /* namespace alinous */
