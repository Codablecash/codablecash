/*
 * AbstractConsensusNodeCommand.h
 *
 *  Created on: 2024/05/01
 *      Author: iizuka
 */

#ifndef BC_P2P_CMD_NODE_CONSENSUS_ABSTRACTCONSENSUSNODECOMMAND_H_
#define BC_P2P_CMD_NODE_CONSENSUS_ABSTRACTCONSENSUSNODECOMMAND_H_

#include "bc_p2p_cmd_node/AbstractNodeCommand.h"

#include "base/ArrayList.h"

namespace alinous {
class SystemTimestamp;
}
using namespace alinous;


namespace codablecash {

class NodeNetworkInfo;
class NodeIdentifier;

class AbstractConsensusNodeCommand : public AbstractNodeCommand {
public:
	AbstractConsensusNodeCommand(const AbstractConsensusNodeCommand& inst);
	AbstractConsensusNodeCommand(uint32_t type);
	virtual ~AbstractConsensusNodeCommand();

	virtual SystemTimestamp getFirstTimestamp() const = 0;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;
	virtual void fromBinary(ByteBuffer* buff);

	virtual bool validateCommand(CodablecashNodeInstance* inst, bool suspend) const;


	void addHistory(uint16_t zone, const NodeIdentifier* nodeId, int protocol, const UnicodeString* host, uint32_t port);
	void addHistory(uint16_t zone, const NodeIdentifier* nodeId, const SystemTimestamp* timestamp, int protocol, const UnicodeString* host, uint32_t port);

	const ArrayList<NodeNetworkInfo>* getHistory() const noexcept {
		return this->history;
	}

	void makeHistoryExcludeList(ArrayList<NodeIdentifier>* list) const;

protected:
	virtual bool validateHistory(const NodeNetworkInfo* his, const SystemTimestamp* lastTm, CodablecashNodeInstance *inst) const noexcept;
	virtual bool validateCurrentTime(const SystemTimestamp* lastTm, CodablecashNodeInstance *inst) const noexcept;

protected:
	ArrayList<NodeNetworkInfo>* history;
};

} /* namespace codablecash */

#endif /* BC_P2P_CMD_NODE_CONSENSUS_ABSTRACTCONSENSUSNODECOMMAND_H_ */
