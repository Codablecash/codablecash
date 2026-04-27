/*
 * BlockHeader.h
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#ifndef BC_BLOCK_BLOCKHEADER_H_
#define BC_BLOCK_BLOCKHEADER_H_

#include <cstdint>

#include "filestore_block/IBlockObject.h"

namespace alinous {
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class BlockHeaderId;
class BlockMerkleRoot;
class PoWNonce;
class VotePart;
class VoteBlockTransaction;
class Block;
class IVoteTransactionIdCertificateBuilder;
class IVoteTransactionIdCertificatevisitor;
class TransactionId;

class BlockHeader : public alinous::IBlockObject {
public:
	BlockHeader(const BlockHeader& inst) = delete;
	BlockHeader();
	virtual ~BlockHeader();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static BlockHeader* createFromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	void setHeight(uint64_t height) noexcept {
		this->height = height;
	}
	uint64_t getHeight() const noexcept {
		return this->height;
	}

	void setZone(uint16_t zone) noexcept {
		this->zone = zone;
	}
	uint16_t getZone() const noexcept {
		return this->zone;
	}

	const BlockHeaderId* getId() const noexcept {
		return this->id;
	}
	void setHeaderId(BlockHeaderId* id) noexcept;
	void buildHeaderId();

	void incTimestamp() noexcept;

	void setTimestamp(const SystemTimestamp* tm);
	const SystemTimestamp* getTimestamp() const noexcept {
		return this->timestamp;
	}

	const SystemTimestamp* getNonceGeneratedtimestamp() const noexcept {
		return this->nonceGeneratedtimestamp;
	}
	void setNonceGeneratedTimestamp(const SystemTimestamp* tm) const noexcept;

	void setMerkleRoot(const BlockMerkleRoot* merkleRoot);
	const BlockMerkleRoot* getMerkleRoot() const noexcept {
		return this->merkleRoot;
	}

	const PoWNonce* getPoWNonce() const noexcept {
		return this->nonce;
	}
	void setPoWNonce(const PoWNonce* nonce) noexcept;

	bool isScheduledBlock() const noexcept;

	const BlockHeaderId* getLastHeaderId() const noexcept {
		return this->lastid;
	}
	void setLastHeaderId(const BlockHeaderId* lastid) noexcept;

	void addVote(const VoteBlockTransaction* trx) noexcept;

	bool isFinalizing(int votePerBlock) const noexcept;

	VotePart* getVotePart() const noexcept {
		return this->votePart;
	}

	void setLastNouceCalculated(const SystemTimestamp* tm) noexcept;

	void buildVoteTransactionIdCertificate(const Block* block, IVoteTransactionIdCertificateBuilder* callback) const;
	void visitVoteTransactionIdCertificate(IVoteTransactionIdCertificatevisitor* visitor) const;

private:
	uint16_t zone;
	uint64_t height;
	SystemTimestamp* timestamp;
	/**
	 * supported the accuracy by network transfer protocol
	 */
	SystemTimestamp* nonceGeneratedtimestamp;
	BlockHeaderId* id; // built by buildHeaderId()

	BlockMerkleRoot* merkleRoot;

	BlockHeaderId* lastid;
	PoWNonce* nonce;

	VotePart* votePart;
	SystemTimestamp* lastNouceCalculated;
};

class IVoteTransactionIdCertificateBuilder {
public:
	virtual ~IVoteTransactionIdCertificateBuilder(){}

	virtual void detect(const Block* block, BlockHeaderId* votedHeaderId, TransactionId* voteTrxId) = 0;
};

class IVoteTransactionIdCertificatevisitor {
public:
	virtual ~IVoteTransactionIdCertificatevisitor(){}

	virtual void visit(const BlockHeaderId* votedHeaderId, const TransactionId* voteTrxId) = 0;
};


} /* namespace codablecash */

#endif /* BC_BLOCK_BLOCKHEADER_H_ */
