/*
 * BlockHeaderHeightIndexKey.cpp
 *
 *  Created on: 2023/02/16
 *      Author: iizuka
 */

#include "bc_blockstore_header/BlockHeaderHeightIndexKey.h"
#include "bc_blockstore_header/BlockHeaderHeightIndexKeyFactory.h"


namespace codablecash {

BlockHeaderHeightIndexKey::BlockHeaderHeightIndexKey(uint64_t value) {
	this->value = value;
	this->fpos = 0;
}

BlockHeaderHeightIndexKey::BlockHeaderHeightIndexKey(uint64_t value, uint64_t fpos) {
	this->value = value;
	this->fpos = fpos;
}

BlockHeaderHeightIndexKey::~BlockHeaderHeightIndexKey() {

}

int BlockHeaderHeightIndexKey::binarySize() const {
	int size = sizeof(uint32_t);
	size += sizeof(this->value) + sizeof(this->fpos);
	return size;
}

void BlockHeaderHeightIndexKey::toBinary(ByteBuffer *out) const {
	out->putInt(BlockHeaderHeightIndexKeyFactory::HEIGHT_INDEX_KEY);
	out->putLong(this->value);
	out->putLong(this->fpos);
}

BlockHeaderHeightIndexKey* BlockHeaderHeightIndexKey::fromBinary(ByteBuffer *in) {
	uint64_t value = in->getLong();
	uint64_t fpos = in->getLong();

	return new BlockHeaderHeightIndexKey(value, fpos);
}

int BlockHeaderHeightIndexKey::compareTo(const AbstractBtreeKey *key) const noexcept {
	if(key->isInfinity()){
		return -1;
	}
	else if(key->isNull()){
		return 1;
	}

	const BlockHeaderHeightIndexKey* ulkey = dynamic_cast<const BlockHeaderHeightIndexKey*>(key);

	return this->value == ulkey->value ? 0 : (this->value > ulkey->value ? 1 : -1);
}

AbstractBtreeKey* BlockHeaderHeightIndexKey::clone() const noexcept {
	return new BlockHeaderHeightIndexKey(this->value, this->fpos);
}

} /* namespace codablecash */
