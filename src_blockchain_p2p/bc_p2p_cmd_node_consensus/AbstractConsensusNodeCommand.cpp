/*
 * AbstractConsensusNodeCommand.cpp
 *
 *  Created on: 2024/05/01
 *      Author: iizuka
 */


#include "bc_p2p_cmd_node_consensus/AbstractConsensusNodeCommand.h"
#include "base_timestamp/SystemTimestamp.h"

#include "bc/CodablecashSystemParam.h"
#include "bc/CodablecashNodeInstance.h"
#include "bc_p2p_cmd_network/NodeNetworkInfo.h"

namespace codablecash {


AbstractConsensusNodeCommand::AbstractConsensusNodeCommand(const AbstractConsensusNodeCommand &inst)
		: AbstractNodeCommand(inst) {
	this->history = new ArrayList<NodeNetworkInfo>();

	int maxLoop = inst.history->size();
	for(int i = 0; i != maxLoop; ++i){
		NodeNetworkInfo* nhis = inst.history->get(i);

		this->history->addElement(new NodeNetworkInfo(*nhis));
	}
}

AbstractConsensusNodeCommand::AbstractConsensusNodeCommand(uint32_t type) : AbstractNodeCommand(type) {
	this->history = new ArrayList<NodeNetworkInfo>();

}

AbstractConsensusNodeCommand::~AbstractConsensusNodeCommand() {
	this->history->deleteElements();
	delete this->history;
}

bool AbstractConsensusNodeCommand::validateCommand(CodablecashNodeInstance *inst, bool suspend) const {
	SystemTimestamp firstTm = getFirstTimestamp();
	SystemTimestamp tm = firstTm;

	// [Consensus] firstTm must be more than now
	SystemTimestamp tmnow;
	if(firstTm.compareTo(&tmnow) > 0){
		return false;
	}

	int maxLoop = this->history->size();
	for(int i = 0; i != maxLoop; ++i){
		NodeNetworkInfo* his = this->history->get(i);
		const SystemTimestamp* htm = his->getTimestamp();

		if(!validateHistory(his, &tm, inst)){
			return false;
		}

		tm = *htm;
	}

	return validateCurrentTime(&tm, inst);
}

int AbstractConsensusNodeCommand::binarySize() const {
	int total = AbstractNodeCommand::binarySize();

	int maxLoop = this->history->size();
	total += sizeof(uint32_t);

	for(int i = 0; i != maxLoop; ++i){
		NodeNetworkInfo* his = this->history->get(i);

		total += his->binarySize();
	}


	return total;
}

void AbstractConsensusNodeCommand::toBinary(ByteBuffer *buff) const {
	AbstractNodeCommand::toBinary(buff);

	int maxLoop = this->history->size();
	buff->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		NodeNetworkInfo* his = this->history->get(i);

		his->toBinary(buff);
	}
}

void AbstractConsensusNodeCommand::fromBinary(ByteBuffer *buff) {
	AbstractNodeCommand::fromBinary(buff);

	int maxLoop = buff->getInt();

	for(int i = 0; i != maxLoop; ++i){
		NodeNetworkInfo* his = NodeNetworkInfo::fromBinary(buff);

		this->history->addElement(his);
	}

}

void AbstractConsensusNodeCommand::addHistory(uint16_t zone, const NodeIdentifier *nodeId, int protocol, const UnicodeString *host, uint32_t port) {
	SystemTimestamp tm;
	addHistory(zone, nodeId, &tm, protocol, host, port);
}

void AbstractConsensusNodeCommand::addHistory(uint16_t zone, const NodeIdentifier *nodeId, const SystemTimestamp *timestamp,
		int protocol, const UnicodeString *host, uint32_t port) {
	NodeNetworkInfo* his = new NodeNetworkInfo(zone, nodeId, timestamp, protocol, host, port);
	this->history->addElement(his);
}

bool AbstractConsensusNodeCommand::validateHistory(const NodeNetworkInfo *his, const SystemTimestamp *lastTm, CodablecashNodeInstance *inst) const noexcept {
	const CodablecashSystemParam* params = inst->getCodablecashSystemParam();
	uint32_t consensusTrxAllowedDelayMillis = params->getConsensusTrxAllowedDelayMillis();

	const SystemTimestamp* thisTm = his->getTimestamp();
	SystemTimestamp mills(0, consensusTrxAllowedDelayMillis * 1000);
	SystemTimestamp lim = *lastTm + mills;

	return lim.compareTo(thisTm) >= 0;
}

bool AbstractConsensusNodeCommand::validateCurrentTime(const SystemTimestamp *lastTm, CodablecashNodeInstance *inst) const noexcept {
	const CodablecashSystemParam* params = inst->getCodablecashSystemParam();
	uint32_t consensusTrxAllowedDelayMillis = params->getConsensusTrxAllowedDelayMillis();
	uint16_t maxHops = params->getConsensusTrxMaxHops();

	SystemTimestamp tmnow;
	SystemTimestamp mills(0, consensusTrxAllowedDelayMillis * 1000);
	SystemTimestamp lim = *lastTm + mills;

	int hops = this->history->size() + 1;

	return hops <= maxHops && lim.compareTo(&tmnow) >= 0;
}

} /* namespace codablecash */
