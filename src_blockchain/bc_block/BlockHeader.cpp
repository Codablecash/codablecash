/*
 * BlockHeader.cpp
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"
#include "bc_block/BlockVersion.h"

#include "bc_block_header_command/AbstractBlockHeaderCommand.h"

#include "base/StackRelease.h"

#include "base_timestamp/SystemTimestamp.h"

#include "crypto/Sha256.h"

#include "osenv/funcs.h"

#include "bc_block/BlockMerkleRoot.h"

#include "pow/PoWNonce.h"

#include "numeric/BigInteger.h"

#include "bc_block_vote/VotePart.h"
#include "bc_block_vote/VotedHeaderIdGroup.h"

#include "bc_base/BinaryUtils.h"

#include "bc_block_header_command/BlockHeaderCommandId.h"

namespace codablecash {

BlockHeader::BlockHeader() {
	this->version = new BlockVersion(1, 0, 0);
	this->zone = 0;
	this->height = 0;
	this->id = new BlockHeaderId();

	this->timestamp = dynamic_cast<SystemTimestamp*>(Os::now().copyData());
	this->nonceGeneratedtimestamp = dynamic_cast<SystemTimestamp*>(this->timestamp->copyData());
	this->merkleRoot = BlockMerkleRoot::createZeroRoot();

	uint8_t tmp[32];
	Mem::memset(tmp, 0, 32);
	this->lastid = new BlockHeaderId((const char*)tmp, sizeof(tmp));

	UnicodeString f(L"FFFF");
	BigInteger defaultNonce = PoWNonce::getMaxBigInt()->subtract(BigInteger(f, 16));
	this->nonce = new PoWNonce(&defaultNonce);

	this->votePart = new VotePart();
	this->lastNouceCalculated = dynamic_cast<SystemTimestamp*>(Os::now().copyData());

	this->commnads = new ArrayList<AbstractBlockHeaderCommand>();
}

BlockHeader::~BlockHeader() {
	delete this->version;
	delete this->id;
	delete this->merkleRoot;
	delete this->lastid;
	delete this->nonce;
	delete this->votePart;

	delete this->timestamp;
	delete this->nonceGeneratedtimestamp;
	delete this->lastNouceCalculated;

	this->commnads->deleteElements();
	delete this->commnads;
}

int BlockHeader::binarySize() const {
	BinaryUtils::checkNotNull(this->version);

	int total = this->version->binarySize() + sizeof(this->zone) + sizeof(this->height);

	total += this->timestamp->binarySize();
	total += this->nonceGeneratedtimestamp->binarySize();
	total += this->merkleRoot->binarySize();
	total += this->lastid->binarySize();
	total += this->nonce->binarySize();
	total += this->votePart->binarySize();
	total += this->lastNouceCalculated->binarySize();

	total += sizeof(uint8_t);
	int maxLoop = this->commnads->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockHeaderCommand* cmd = this->commnads->get(i);
		total += cmd->binarySize();
	}

	return total;
}

void BlockHeader::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->version);

	this->version->toBinary(out);
	out->putShort(this->zone);
	out->putLong(this->height);

	this->timestamp->toBinary(out);
	this->nonceGeneratedtimestamp->toBinary(out);
	this->merkleRoot->toBinary(out);
	this->lastid->toBinary(out);
	this->nonce->toBinary(out);
	this->votePart->toBinary(out);
	this->lastNouceCalculated->toBinary(out);

	int maxLoop = this->commnads->size();
	out->put(maxLoop);
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockHeaderCommand* cmd = this->commnads->get(i);
		cmd->toBinary(out);
	}
}

BlockHeader* BlockHeader::createFromBinary(ByteBuffer* in) {
	BlockHeader* header = new BlockHeader(); __STP(header);

	BlockVersion* ver = BlockVersion::createFromBinary(in); __STP(ver);
	header->setVersion(ver);

	uint16_t zone = in->getShort();
	header->setZone(zone);

	uint64_t height = in->getLong();
	header->setHeight(height);

	delete header->timestamp;
	header->timestamp = SystemTimestamp::fromBinary(in);

	delete header->nonceGeneratedtimestamp;
	header->nonceGeneratedtimestamp = SystemTimestamp::fromBinary(in);

	BlockMerkleRoot* root = BlockMerkleRoot::fromBinary(in); __STP(root);
	header->setMerkleRoot(root);

	delete header->lastid;
	header->lastid = BlockHeaderId::fromBinary(in);

	delete header->nonce;
	header->nonce = PoWNonce::createFromBinary(in);

	delete header->votePart;
	header->votePart = VotePart::createFromBinary(in);

	delete header->lastNouceCalculated;
	header->lastNouceCalculated = SystemTimestamp::fromBinary(in);

	int maxLoop = in->get();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockHeaderCommand* cmd = AbstractBlockHeaderCommand::createFromBinary(in);
		BinaryUtils::checkNotNull(cmd);

		header->commnads->addElement(cmd);
	}

	header->buildHeaderId();

	return __STP_MV(header);
}

IBlockObject* BlockHeader::copyData() const noexcept {
	int size = binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(size, true); __STP(buff);
	toBinary(buff);

	buff->position(0);
	return BlockHeader::createFromBinary(buff);
}

void BlockHeader::setVersion(const BlockVersion *ver) {
	delete this->version, this->version = nullptr;
	this->version = new BlockVersion(*ver);
}

void BlockHeader::setHeaderId(BlockHeaderId* id) noexcept {
	delete this->id;
	this->id = id;
}

void BlockHeader::buildHeaderId() {
	BinaryUtils::checkNotNull(this->version);

	int total = 0;
	{
		total += this->version->binarySize() + sizeof(this->zone) + sizeof(this->height);
		total += this->timestamp->binarySize();
		//total += this->nonceGeneratedtimestamp->binarySize(); // do not include calculated time

		total += this->merkleRoot->binarySize();
		total += this->lastid->binarySize();
		// total += this->nonce->binarySize();
		total += this->votePart->binarySize();
		total += this->lastNouceCalculated->binarySize();

		total += sizeof(uint8_t);
		int maxLoop = this->commnads->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractBlockHeaderCommand* cmd = this->commnads->get(i);
			total += cmd->binarySize();
		}
	}

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(total, true); __STP(buff);
	this->version->toBinary(buff);
	buff->putShort(this->zone);
	buff->putLong(this->height);
	this->timestamp->toBinary(buff);
	//this->nonceGeneratedtimestamp->toBinary(buff);

	this->merkleRoot->toBinary(buff);
	this->lastid->toBinary(buff);
	// this->nonce->toBinary(buff);
	this->votePart->toBinary(buff);
	this->lastNouceCalculated->toBinary(buff);

	int maxLoop = this->commnads->size();
	buff->put(maxLoop);
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockHeaderCommand* cmd = this->commnads->get(i);
		cmd->toBinary(buff);
	}

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);
	sha->position(0);
	BlockHeaderId* newId = new BlockHeaderId((const char*)sha->array(), sha->limit());

	setHeaderId(newId);
}

void BlockHeader::setMerkleRoot(const BlockMerkleRoot *merkleRoot) {
	delete this->merkleRoot;
	this->merkleRoot = dynamic_cast<BlockMerkleRoot*>(merkleRoot->copyData());
}

void BlockHeader::setLastHeaderId(const BlockHeaderId *lastid) noexcept {
	delete this->lastid;
	this->lastid = dynamic_cast<BlockHeaderId*>(lastid->copyData());
}

void BlockHeader::setPoWNonce(const PoWNonce *nonce) noexcept {
	delete this->nonce;
	this->nonce = dynamic_cast<PoWNonce*>(nonce->copyData());
}

void BlockHeader::addVote(const VoteBlockTransaction *trx) noexcept {
	this->votePart->addVote(trx);
}

bool BlockHeader::isFinalizing(int votePerBlock) const noexcept {
	const VotedHeaderIdGroup* group = this->votePart->getMaxVotedGroup();

	return group != nullptr && group->size() == votePerBlock;
}

void BlockHeader::buildVoteTransactionIdCertificate(const Block *block, IVoteTransactionIdCertificateBuilder *callback) const {
	if(this->votePart != nullptr){
		this->votePart->buildVoteTransactionIdCertificate(block, callback);
	}
}

void BlockHeader::visitVoteTransactionIdCertificate(IVoteTransactionIdCertificatevisitor *visitor) const {
	if(this->votePart != nullptr){
		this->votePart->visitVoteTransactionIdCertificate(visitor);
	}
}

void BlockHeader::incTimestamp() noexcept {
	SystemTimestamp tm(1, 0);

	*this->timestamp = *this->timestamp + tm;
}

void BlockHeader::setTimestamp(const SystemTimestamp *tm) {
	*this->timestamp = *tm;
}

void BlockHeader::setNonceGeneratedTimestamp(const SystemTimestamp *tm) const noexcept {
	*this->nonceGeneratedtimestamp = *tm;
}

bool BlockHeader::isScheduledBlock() const noexcept {
	return this->nonce->compareTo(&PoWNonce::MAX_NONCE) == 0;
}

void BlockHeader::setLastNouceCalculated(const SystemTimestamp *tm) noexcept {
	delete this->lastNouceCalculated;
	this->lastNouceCalculated = dynamic_cast<SystemTimestamp*>(tm->copyData());
}

void BlockHeader::addHeaderCommand(const AbstractBlockHeaderCommand *cmd) {
	this->commnads->addElement(dynamic_cast<AbstractBlockHeaderCommand*>(cmd->copyData()));
}

bool BlockHeader::hasHeaderCommnads() const noexcept {
	return !this->commnads->isEmpty();
}

const AbstractBlockHeaderCommand* BlockHeader::getHeaderCommand(const BlockHeaderCommandId *commandId) const noexcept {
	const AbstractBlockHeaderCommand* ret = nullptr;

	int maxLoop = this->commnads->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockHeaderCommand* command = this->commnads->get(i);
		const BlockHeaderCommandId* id = command->getCommandId();

		if(commandId->equals(id)){
			ret = command;
			break;
		}
	}

	return ret;
}

} /* namespace codablecash */
