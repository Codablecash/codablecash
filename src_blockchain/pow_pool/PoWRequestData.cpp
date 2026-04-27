/*
 * PoWRequestData.cpp
 *
 *  Created on: Apr 14, 2026
 *      Author: iizuka
 */

#include "pow_pool/PoWRequestData.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/BlockMerkleRoot.h"

#include "base_timestamp/SystemTimestamp.h"

#include "numeric/BigInteger.h"

#include "base/StackRelease.h"

#include "bc_base/BinaryUtils.h"
using alinous::StackArrayRelease;

namespace codablecash {

PoWRequestData::PoWRequestData(const PoWRequestData &inst) {
	this->lastBlockHeight = inst.lastBlockHeight;
	this->lastBlockId = inst.lastBlockId != nullptr ? dynamic_cast<BlockHeaderId*>(inst.lastBlockId->copyData()) : nullptr;
	this->merkleRoot = inst.merkleRoot != nullptr ? dynamic_cast<BlockMerkleRoot*>(inst.merkleRoot->copyData()) : nullptr;
	this->tm = inst.tm != nullptr ? new SystemTimestamp(*inst.tm) : nullptr;
	this->targetDifficulty = inst.targetDifficulty != nullptr ? new BigInteger(*inst.targetDifficulty) : nullptr;
}

PoWRequestData::PoWRequestData() {
	this->lastBlockHeight = 0;
	this->lastBlockId = nullptr;
	this->merkleRoot = nullptr;
	this->tm = nullptr;
	this->targetDifficulty = nullptr;

}

PoWRequestData::~PoWRequestData() {
	delete this->lastBlockId;
	delete this->merkleRoot;
	delete this->tm;
	delete this->targetDifficulty;
}

void PoWRequestData::init(uint64_t lastBlockHeight, const BlockHeaderId *lastBlockId, const BlockMerkleRoot *merkleRoot,
		const SystemTimestamp *tm, const BigInteger *targetDifficulty) {
	delete this->lastBlockId;
	delete this->merkleRoot;
	delete this->tm;
	delete this->targetDifficulty;

	this->lastBlockHeight = lastBlockHeight;
	this->lastBlockId = dynamic_cast<BlockHeaderId*>(lastBlockId->copyData());
	this->merkleRoot = dynamic_cast<BlockMerkleRoot*>(merkleRoot->copyData());
	this->tm = dynamic_cast<SystemTimestamp*>(tm->copyData());
	this->targetDifficulty = new BigInteger(*targetDifficulty);
}

int PoWRequestData::binarySize() const {
	BinaryUtils::checkNotNull(this->lastBlockId);
	BinaryUtils::checkNotNull(this->merkleRoot);
	BinaryUtils::checkNotNull(this->tm);
	BinaryUtils::checkNotNull(this->targetDifficulty);

	int total = sizeof(uint64_t); // lastBlockHeight
	total += this->lastBlockId->binarySize();
	total += this->merkleRoot->binarySize();
	total += this->tm->binarySize();

	total += sizeof(uint32_t);
	total += this->targetDifficulty->binarySize();

	return total;
}

void PoWRequestData::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->lastBlockId);
	BinaryUtils::checkNotNull(this->merkleRoot);
	BinaryUtils::checkNotNull(this->tm);
	BinaryUtils::checkNotNull(this->targetDifficulty);

	out->putLong(this->lastBlockHeight);
	this->lastBlockId->toBinary(out);
	this->merkleRoot->toBinary(out);
	this->tm->toBinary(out);

	ByteBuffer* diffb = this->targetDifficulty->toBinary(); __STP(diffb);
	diffb->position(0);

	int length = diffb->limit();
	out->putInt(length);
	out->put(diffb);
}

PoWRequestData* PoWRequestData::fromBinary(ByteBuffer *in) {
	uint64_t lastBlockHeight = in->getLong();
	BlockHeaderId* lastBlockId = BlockHeaderId::fromBinary(in); __STP(lastBlockId);
	BlockMerkleRoot* merkleRoot = BlockMerkleRoot::fromBinary(in); __STP(merkleRoot);
	SystemTimestamp *tm = SystemTimestamp::fromBinary(in); __STP(tm);

	int length = in->getInt();

	uint8_t* buff = new uint8_t[length]; StackArrayRelease<uint8_t> __buff(buff);
	in->get(buff, length);
	BigInteger* targetDifficulty = BigInteger::fromBinary((const char*)buff, length); __STP(targetDifficulty);

	PoWRequestData* ret = new PoWRequestData(); __STP(ret);
	ret->init(lastBlockHeight, lastBlockId, merkleRoot, tm, targetDifficulty);

	return __STP_MV(ret);
}

} /* namespace codablecash */
