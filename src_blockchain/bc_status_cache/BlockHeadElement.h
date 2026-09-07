/*
 * BlockHeadElement.h
 *
 *  Created on: 2023/05/19
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_BLOCKHEADELEMENT_H_
#define BC_STATUS_CACHE_BLOCKHEADELEMENT_H_

namespace alinous {
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class BlockHeader;
class VotePart;
class BlockHeaderId;


class BlockHeadElement {
public:
	BlockHeadElement(const BlockHeadElement& inst);
	explicit BlockHeadElement(const BlockHeader* header);
	virtual ~BlockHeadElement();

	const BlockHeader* getBlockHeader() const noexcept {
		return this->header;
	}

	void importVotes(const VotePart* votes) noexcept;

	void hasTransactionInMemoryPool() noexcept;
	void hasNoTransactionInMemoryPool() noexcept;

	//int compareTo(const BlockHeadElement* other) const noexcept;

	UnicodeString* toString() const noexcept;

	int getVotedScore() const noexcept {
		return this->votedScore;
	}
	void setVotedScore(int score) noexcept {
		this->votedScore = score;
	}

	int getVotingSocre() const noexcept {
		return this->votingScore;
	}
	void setVotingScore(int votingScore) {
		this->votingScore = votingScore;
	}

	int getMevHealthScore() const noexcept {
		return this->mevHealthScore;
	}
	void setMevHealthScore(int score) noexcept {
		this->mevHealthScore = score;
	}

	void calcVotingScore(const BlockHeaderId* votedId);

	virtual bool isPaddong() const noexcept {
		return false;
	}

protected:
	BlockHeader* header;

	int votedScore;
	int votingScore;
	int mevHealthScore;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_BLOCKHEADELEMENT_H_ */
