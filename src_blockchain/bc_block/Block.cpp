/*
 * Block.cpp
 *
 *  Created on: 2023/03/17
 *      Author: iizuka
 */

#include "bc_block/Block.h"
#include "bc_block/BlockHeader.h"
#include "bc_block/BlockMerkleRoot.h"

#include "bc_block_body/BlockBody.h"

#include "bc_trx/AbstractControlTransaction.h"

#include "bc_finalizer_trx/VoteBlockTransaction.h"

#include "osenv/funcs.h"

#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"

#include "base_timestamp/SystemTimestamp.h"
using namespace alinous;

namespace codablecash {

Block::Block(const Block &inst) {
	this->header = dynamic_cast<BlockHeader*>(inst.header->copyData());
	this->body = dynamic_cast<BlockBody*>(inst.body->copyData());
}

Block::Block(uint16_t zone, uint64_t height) {
	uint64_t sec = Os::getMicroSec();

	this->header = new BlockHeader();
	this->body = new BlockBody(sec);

	this->header->setZone(zone);
	this->header->setHeight(height);
}

Block::Block(BlockHeader *header, BlockBody *body) {
	this->header = header;
	this->body = body;
}

Block::~Block() {
	delete this->header;
	delete this->body;
}

void Block::addBalanceTransaction(const AbstractBalanceTransaction *trx) noexcept {
	this->body->addBalanceTransaction(trx);
}

void Block::addControlTransaction(const AbstractControlTransaction *trx) noexcept {
	uint8_t type = trx->getType();

	switch(type){
	case AbstractBlockchainTransaction::TRX_TYPE_VOTE_BLOCK:
		addVote(dynamic_cast<const VoteBlockTransaction*>(trx));
		break;
	// FIXME[multishard]add header command to register new zone
	default:
		this->body->addControlTransaction(trx);
		break;
	}
}

void Block::addVote(const VoteBlockTransaction *votetrx) {
	this->body->addControlTransaction(votetrx);

	this->header->addVote(votetrx);
}

void Block::addInterChainCommunicationTransaction(const AbstractInterChainCommunicationTansaction *trx) noexcept {
	this->body->addInterChainCommunicationTransaction(trx);
}

void Block::addSmartcontractTransaction(const AbstractSmartcontractTransaction *trx) noexcept {
	this->body->addSmartcontractTransaction(trx);
}

void Block::build() {
	this->body->build();
	const BlockMerkleRoot* merkleRoot = this->body->getMerkleRoot();

	this->header->setMerkleRoot(merkleRoot);
	this->header->buildHeaderId();
}

uint16_t Block::getZone() const noexcept {
	return this->header->getZone();
}

uint64_t Block::getHeight() const noexcept {
	return this->header->getHeight();
}

void Block::setLastHeaderId(const BlockHeaderId *lastHeaderId) noexcept {
	this->header->setLastHeaderId(lastHeaderId);
}

void Block::setPoWNonce(const PoWNonce *nonce) noexcept {
	this->header->setPoWNonce(nonce);
}

bool Block::isFinalizing(int votePerBlock) const noexcept {
	return this->header->isFinalizing(votePerBlock);
}

int Block::binarySize() const {
	BinaryUtils::checkNotNull(this->header);
	BinaryUtils::checkNotNull(this->body);

	int total = this->header->binarySize();
	total += this->body->binarySize();

	return total;
}

void Block::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->header);
	BinaryUtils::checkNotNull(this->body);

	this->header->toBinary(buff);
	this->body->toBinary(buff);
}

Block* Block::createFromBinary(ByteBuffer *buff) {
	BlockHeader *header = BlockHeader::createFromBinary(buff); __STP(header);
	BinaryUtils::checkNotNull(header);

	BlockBody *body = BlockBody::fromBinary(buff); __STP(body);
	BinaryUtils::checkNotNull(body);

	return new Block(__STP_MV(header), __STP_MV(body));
}

#ifdef __DEBUG__
bool Block::checkMerkleRoot() const noexcept {
	const BlockMerkleRoot* rootheader = this->header->getMerkleRoot();
	const BlockMerkleRoot* rootbody = this->body->getMerkleRoot();

	bool bl = rootheader->equals(rootbody);
	return bl;
}

#endif

const SystemTimestamp* Block::getTimestamp() const noexcept {
	return this->header->getTimestamp();
}

void Block::setNonceGeneratedTimestamp(const SystemTimestamp *tm) const noexcept {
	this->header->setNonceGeneratedTimestamp(tm);
}

void Block::setGenesisTimestamp() {
	SystemTimestamp tm = Os::now();
	SystemTimestamp diff(3600*24, 0);

	tm = tm - diff;
	this->header->setTimestamp(&tm);
}

bool Block::isScheduledBlock() const noexcept {
	return this->header->isScheduledBlock();
}

} /* namespace codablecash */
