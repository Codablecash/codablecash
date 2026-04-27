/*
 * BlockHeadElement.cpp
 *
 *  Created on: 2023/05/19
 *      Author: iizuka
 */

#include "bc_status_cache/BlockHeadElement.h"

#include "bc_block/BlockHeader.h"

#include "bc_block_vote/VotedHeaderIdGroup.h"
#include "bc_block_vote/VotePart.h"

#include "base/StackRelease.h"

#include "bc_block/BlockHeaderId.h"
namespace codablecash {

BlockHeadElement::BlockHeadElement(const BlockHeadElement &inst) {
	this->header = inst.header != nullptr ? dynamic_cast<BlockHeader*>(inst.header->copyData()) : nullptr;

	this->votingScore = inst.votingScore;
	this->votedScore = inst.votedScore;
	this->mevHealthScore = inst.mevHealthScore;
}

BlockHeadElement::BlockHeadElement(const BlockHeader* header) {
	this->header = header != nullptr ? dynamic_cast<BlockHeader*>(header->copyData()) : nullptr;

	this->votingScore = 0;
	this->votedScore = 0;
	this->mevHealthScore = 0;
}

BlockHeadElement::~BlockHeadElement() {
	delete this->header;
}

void BlockHeadElement::importVotes(const VotePart *vorts) noexcept {
	const BlockHeaderId* id = this->header->getId();

	const HashMap<BlockHeaderId, VotedHeaderIdGroup>* map = vorts->getMap();

	Iterator<BlockHeaderId>* it = map->keySet()->iterator(); __STP(it);
	while(it->hasNext()){
		const BlockHeaderId* headerId  = it->next();

		if(id->compareTo(headerId) == 0){
			VotedHeaderIdGroup* group = map->get(headerId);
			this->votedScore += group->size();
		}
	}
}

void BlockHeadElement::hasTransactionInMemoryPool() noexcept {
	++this->mevHealthScore;
}

void BlockHeadElement::hasNoTransactionInMemoryPool() noexcept {
	--this->mevHealthScore;
}

/*
int BlockHeadElement::compareTo(const BlockHeadElement *other) const noexcept {
	int diff = this->votedScore - other->votedScore;
	if(diff != 0){
		return diff;
	}

	diff = this->mevHealthScore - other->mevHealthScore;
	return diff;
}
*/

UnicodeString* BlockHeadElement::toString() const noexcept {
	UnicodeString* id = this->header->getId()->toString(); __STP(id);
	uint64_t height = this->header->getHeight();

	UnicodeString* message = new UnicodeString(id);
	message->append(L" [");

	message->append(L"height: ").append((int64_t)height);
	message->append(L" voted: ").append((int64_t)this->votedScore);
	message->append(L" voting: ").append((int64_t)this->votingScore);
	message->append(L" mev: ").append((int64_t)this->mevHealthScore);

	message->append(L"]");

	return message;
}

void BlockHeadElement::calcVotingScore(const BlockHeaderId *votedId) {
	VotePart* votePart = this->header->getVotePart();

	const VotedHeaderIdGroup* group = votePart->getVotedGroup(votedId);
	if(group != nullptr){
		this->votingScore = group->size();
	}
}

} /* namespace codablecash */
