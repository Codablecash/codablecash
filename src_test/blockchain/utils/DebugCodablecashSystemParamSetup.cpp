/*
 * DebugCodablecashConfigSetup.cpp
 *
 *  Created on: 2023/05/04
 *      Author: iizuka
 */

#include "DebugCodablecashSystemParamSetup.h"
#include "bc/CodablecashSystemParam.h"



namespace codablecash {

void DebugCodablecashSystemParamSetup::setupConfig01(CodablecashSystemParam &param) {
	param.setPowHashrateBlocks(10);
	param.setPowBlockTimeMills(250);
	param.setPowAllowedSceduledDiffMevScore(1);

	param.setVoteMissingLimit(1);
	param.setTicketMatureIntervalHeight(2);
	param.setTicketExpireHeight(4);
	param.setVoteExtendCapacityCount(4);

	param.setVoteLockinIntervalHeight(5);

	param.setTicketPriceDefault(2);
	param.setTicketPriceWindow(3);
	param.setTicketIdealNumber(100);

	param.setTicketVoterFeeBasisPoint(50);

	param.setVoteBeforeNBlocks(1);
	param.setVoteBlockIncludeAfterNBlocks(1);

	param.setThreadsNodeTransferProcessor(2);
	param.setThreadsNetworkInfoTransferProcessor(2);

	param.setConsensusTrxMaxHops(3);
	param.setConsensusTrxAllowedDelayMillis(10000);
	param.setConsensusPosVoteLimitMillis(3000);
}

void DebugCodablecashSystemParamSetup::setupConfig02(CodablecashSystemParam &param) {
	param.setPowHashrateBlocks(10);
	param.setPowBlockTimeMills(50);

	param.setVoteMissingLimit(1);
	param.setTicketMatureIntervalHeight(2);
	param.setTicketExpireHeight(4);
	param.setVoteExtendCapacityCount(4);

	param.setVoteLockinIntervalHeight(5);

	param.setTicketPriceDefault(2);
	param.setTicketPriceWindow(3);
	param.setTicketIdealNumber(100);

	param.setTicketVoterFeeBasisPoint(50);

	param.setVoteBeforeNBlocks(1);
	param.setVoteBlockIncludeAfterNBlocks(1);

	param.setThreadsNodeTransferProcessor(2);
	param.setThreadsNetworkInfoTransferProcessor(2);

	param.setNumConnectionOwnShard(2);
	param.setNumConnectionExtShard(2);

	param.setNumInitialSyncNodes(3);

	param.setConsensusTrxMaxHops(3);
	param.setConsensusTrxAllowedDelayMillis(5000);
	param.setConsensusPosVoteLimitMillis(3000);
}


} /* namespace codablecash */
