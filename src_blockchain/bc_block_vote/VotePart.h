/*
 * VotePart.h
 *
 *  Created on: 2023/05/06
 *      Author: iizuka
 */

#ifndef BC_BLOCK_VOTE_VOTEPART_H_
#define BC_BLOCK_VOTE_VOTEPART_H_

#include "base/HashMap.h"

namespace alinous {
class ByteBuffer;
}
using namespace alinous;

namespace codablecash {

class VotedHeaderIdGroup;
class BlockHeaderId;
class VoteBlockTransaction;
class Block;
class IVoteTransactionIdCertificateBuilder;
class IVoteTransactionIdCertificatevisitor;

class VotePart {
public:
	VotePart();
	virtual ~VotePart();

	void addVote(const VoteBlockTransaction* trx) noexcept;

	int binarySize() const;
	void toBinary(ByteBuffer* out) const;
	static VotePart* createFromBinary(ByteBuffer* in);

	const VotedHeaderIdGroup* getMaxVotedGroup() const noexcept;
	const VotedHeaderIdGroup* getVotedGroup(const BlockHeaderId* votedHeaderId) const noexcept;

	const HashMap<BlockHeaderId, VotedHeaderIdGroup>* getMap() const noexcept {
		return this->map;
	}

	void buildVoteTransactionIdCertificate(const Block* block, IVoteTransactionIdCertificateBuilder* callback) const;
	void visitVoteTransactionIdCertificate(IVoteTransactionIdCertificatevisitor* visitor) const;

private:
	ArrayList<BlockHeaderId>* list;
	HashMap<BlockHeaderId, VotedHeaderIdGroup>* map;
};

} /* namespace codablecash */

#endif /* BC_BLOCK_VOTE_VOTEPART_H_ */
