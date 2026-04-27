/*
 * CodablecashConfig.cpp
 *
 *  Created on: 2023/03/30
 *      Author: iizuka
 */

#include "bc/CodablecashSystemParam.h"
#include "cassert"
namespace codablecash {

CodablecashSystemParam::CodablecashSystemParam(const CodablecashSystemParam &inst) {
	this->powHashrateBlocks = inst.powHashrateBlocks;
	this->powBlockTimeMills = inst.powBlockTimeMills;
	this->powAllowedSceduledDiffMevScore = inst.powAllowedSceduledDiffMevScore;

	this->voteDefaultCapacity = inst.voteDefaultCapacity;
	this->voteMissingLimit = inst.voteMissingLimit;
	this->voteExtendCapacityCount = inst.voteDefaultCapacity;

	this->voteLockinIntervalHeight = inst.voteLockinIntervalHeight;

	this->ticketMatureIntervalHeight = inst.ticketMatureIntervalHeight;
	this->ticketExpireHeight = inst.ticketExpireHeight;

	this->ticketPriceDefault = inst.ticketPriceDefault;
	this->ticketPriceWindow = inst.ticketPriceWindow;
	this->ticketIdealNumber = inst.ticketIdealNumber;

	this->ticketVoterFeeBasisPoint = inst.ticketVoterFeeBasisPoint;

	this->voteBeforeNBlocks = inst.voteBeforeNBlocks;
	this->voteBlockIncludeAfterNBlocks = inst.voteBlockIncludeAfterNBlocks;

	this->votePerBlock = inst.votePerBlock;

	this->threadsNodeTransferProcessor = inst.threadsNodeTransferProcessor;
	this->threadsNetworkInfoTransferProcessor = inst.threadsNetworkInfoTransferProcessor;

	this->numConnectionOwnShard = inst.numConnectionOwnShard;
	this->numConnectionExtShard = inst.numConnectionExtShard;

	this->numInitialSyncNodes = inst.numInitialSyncNodes;

	this->consensusTrxMaxHops = inst.consensusTrxMaxHops;
	this->consensusTrxAllowedDelayMillis = inst.consensusTrxAllowedDelayMillis;

	this->consensusPosVoteLimitMillis = inst.consensusPosVoteLimitMillis;
}

CodablecashSystemParam::CodablecashSystemParam() {
	this->powHashrateBlocks = 256;
	this->powBlockTimeMills = 1000 * 60 * 3;
	this->powAllowedSceduledDiffMevScore = 10;

	this->voteDefaultCapacity = 10;
	this->voteMissingLimit = 2;
	this->voteExtendCapacityCount = 10;

	this->voteLockinIntervalHeight = 32;

	this->ticketMatureIntervalHeight = 256;
	this->ticketExpireHeight = 20 * 24 * 30 * 3;

	this->ticketPriceDefault = 2;
	this->ticketPriceWindow = 256;
	this->ticketIdealNumber = 40000;

	this->ticketVoterFeeBasisPoint = 50;

	this->voteBeforeNBlocks = 1;
	this->voteBlockIncludeAfterNBlocks = 1; //

	this->votePerBlock = 5;

	this->threadsNodeTransferProcessor = 8;
	this->threadsNetworkInfoTransferProcessor = 8;

	this->numConnectionOwnShard = 8;
	this->numConnectionExtShard = 4;

	this->numInitialSyncNodes = 3;

	this->consensusTrxMaxHops = 8;
	this->consensusTrxAllowedDelayMillis = 3000;

	this->consensusPosVoteLimitMillis = 20000;
}

CodablecashSystemParam::~CodablecashSystemParam() {

}

uint16_t CodablecashSystemParam::getPowHashrateBlocks(uint64_t height) const noexcept {
	return this->powHashrateBlocks;
}

uint64_t CodablecashSystemParam::getPowBlockTimeMills(uint64_t height) const noexcept {
	return this->powBlockTimeMills;
}

uint16_t CodablecashSystemParam::getPowAllowedSceduledDiffMevScore() const noexcept {
	return this->powAllowedSceduledDiffMevScore;
}

void CodablecashSystemParam::setPowAllowedSceduledDiffMevScore(uint16_t powAllowedSceduledDiffMevScore) noexcept {
	this->powAllowedSceduledDiffMevScore = powAllowedSceduledDiffMevScore;
}

uint16_t CodablecashSystemParam::getVoteDefaultCapacity(uint64_t height) const noexcept {
	return this->voteDefaultCapacity;
}

uint16_t CodablecashSystemParam::getVoteMissingLimit(uint64_t height) const noexcept {
	return this->voteMissingLimit;
}

void CodablecashSystemParam::setVoteMissingLimit(uint16_t missingLimit) noexcept {
	this->voteMissingLimit = missingLimit;
}

uint16_t CodablecashSystemParam::getVoteExtendCapacityCount(uint64_t height) const noexcept {
	return this->voteExtendCapacityCount;
}

void CodablecashSystemParam::setVoteExtendCapacityCount(uint16_t extendCapacityCount) noexcept {
	this->voteExtendCapacityCount = extendCapacityCount;
}

uint16_t CodablecashSystemParam::getVoteLockinIntervalHeight() const noexcept {
	return this->voteLockinIntervalHeight;
}

void CodablecashSystemParam::setVoteLockinIntervalHeight(uint16_t voteLockinIntervalHeight) noexcept {
	this->voteLockinIntervalHeight = voteLockinIntervalHeight;
}

uint16_t CodablecashSystemParam::getTicketMatureIntervalHeight(uint64_t height) const noexcept {
	return this->ticketMatureIntervalHeight;
}

void CodablecashSystemParam::setTicketMatureIntervalHeight(uint16_t tiketMatureIntervalHeight) noexcept {
	this->ticketMatureIntervalHeight = tiketMatureIntervalHeight;
}

uint64_t CodablecashSystemParam::getTicketExpireHeight(uint64_t height) const noexcept {
	return this->ticketExpireHeight;
}

void CodablecashSystemParam::setTicketExpireHeight(uint64_t ticketExpireHeight) noexcept {
	this->ticketExpireHeight = ticketExpireHeight;
}

uint64_t CodablecashSystemParam::getTicketPriceDefault(uint64_t height) const noexcept {
	return this->ticketPriceDefault;
}

void CodablecashSystemParam::setTicketPriceDefault(uint64_t ticketPriceDefault) noexcept {
	this->ticketPriceDefault = ticketPriceDefault;
}

uint64_t CodablecashSystemParam::getTicketPriceWindow(uint64_t height) const noexcept {
	return this->ticketPriceWindow;
}

void CodablecashSystemParam::setTicketPriceWindow(	uint64_t ticketPriceWindow) noexcept {
	this->ticketPriceWindow = ticketPriceWindow;
}

uint64_t CodablecashSystemParam::getTicketIdealNumber(uint64_t height) const noexcept {
	return this->ticketIdealNumber;
}

void CodablecashSystemParam::setTicketIdealNumber(	uint64_t ticketIdealNumber) noexcept {
	this->ticketIdealNumber = ticketIdealNumber;
}

uint16_t CodablecashSystemParam::getVoteBeforeNBlocks(uint64_t height) const noexcept {
	return this->voteBeforeNBlocks;
}

uint64_t CodablecashSystemParam::getTicketVoterFeeBasisPoint(uint64_t height) const noexcept {
	return this->ticketVoterFeeBasisPoint;
}

void CodablecashSystemParam::setTicketVoterFeeBasisPoint(	uint64_t ticketVoterFeeBasisPoint) noexcept {
	this->ticketVoterFeeBasisPoint = ticketVoterFeeBasisPoint;
}


void CodablecashSystemParam::setVoteBeforeNBlocks(uint16_t voteBeforeNBlocks) noexcept {
	this->voteBeforeNBlocks = voteBeforeNBlocks;
}

uint16_t CodablecashSystemParam::getVoteBlockIncludeAfterNBlocks(uint64_t height) const noexcept {
	return this->voteBlockIncludeAfterNBlocks;
}

void CodablecashSystemParam::setVoteBlockIncludeAfterNBlocks(uint16_t voteBlockIncludeAfterNBlocks) noexcept {
	this->voteBlockIncludeAfterNBlocks = voteBlockIncludeAfterNBlocks;
}

void CodablecashSystemParam::setThreadsNodeTransferProcessor(uint8_t threadsNodeTransferProcessor) noexcept {
	this->threadsNodeTransferProcessor = threadsNodeTransferProcessor;
}

void CodablecashSystemParam::setThreadsNetworkInfoTransferProcessor(uint8_t threadsNetworkInfoTransferProcessor) noexcept {
	this->threadsNetworkInfoTransferProcessor = threadsNetworkInfoTransferProcessor;
}

void CodablecashSystemParam::setNumConnectionOwnShard(uint16_t numConnectionOwnShard) noexcept {
	this->numConnectionOwnShard = numConnectionOwnShard;
}

void CodablecashSystemParam::setNumConnectionExtShard(uint16_t numConnectionExtShard) noexcept {
	this->numConnectionExtShard = numConnectionExtShard;
}

void CodablecashSystemParam::setNumInitialSyncNodes(uint8_t numInitialSyncNodes) noexcept {
	this->numInitialSyncNodes = numInitialSyncNodes;
}

void CodablecashSystemParam::setConsensusTrxMaxHops(uint16_t consensusTrxMaxHops) noexcept {
	this->consensusTrxMaxHops = consensusTrxMaxHops;
}

void CodablecashSystemParam::setConsensusTrxAllowedDelayMillis(uint32_t consensusTrxAllowedDelayMillis) noexcept {
	this->consensusTrxAllowedDelayMillis = consensusTrxAllowedDelayMillis;
}

void CodablecashSystemParam::setConsensusPosVoteLimitMillis(uint32_t consensusPosVoteLimitMillis) noexcept {
	this->consensusPosVoteLimitMillis = consensusPosVoteLimitMillis;
}

} /* namespace codablecash */
