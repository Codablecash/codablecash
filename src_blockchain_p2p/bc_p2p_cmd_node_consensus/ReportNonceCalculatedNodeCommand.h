/*
 * ReportNonceCalculatedNodeCommand.h
 *
 *  Created on: 2024/07/23
 *      Author: iizuka
 */

#ifndef BC_P2P_CMD_NODE_CONSENSUS_REPORTNONCECALCULATEDNODECOMMAND_H_
#define BC_P2P_CMD_NODE_CONSENSUS_REPORTNONCECALCULATEDNODECOMMAND_H_

#include "bc_p2p_cmd_node_consensus/AbstractConsensusNodeCommand.h"

namespace codablecash {

class BlockHeader;
class MemoryPool;
class BlockchainController;
class CodablecashSystemParam;

class ReportNonceCalculatedNodeCommand : public AbstractConsensusNodeCommand {
public:
	ReportNonceCalculatedNodeCommand(const ReportNonceCalculatedNodeCommand& inst);
	ReportNonceCalculatedNodeCommand();
	virtual ~ReportNonceCalculatedNodeCommand();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;
	virtual void fromBinary(ByteBuffer* buff);

	virtual ByteBuffer* getSignBinary() const;
	virtual IBlockObject* copyData() const noexcept;

	virtual SystemTimestamp getFirstTimestamp() const;

	void setHeader(const BlockHeader* header);

protected:
	virtual AbstractCommandResponse* executeAsNode(BlockchainNodeHandshake* nodeHandShake, CodablecashNodeInstance* inst, bool suspend) const;

private:
	bool importHeader(MemoryPool* memPool, BlockchainController* ctrl, const BlockHeader* header, CodablecashSystemParam* config) const;

private:
	BlockHeader* header;

};

} /* namespace codablecash */

#endif /* BC_P2P_CMD_NODE_CONSENSUS_REPORTNONCECALCULATEDNODECOMMAND_H_ */
