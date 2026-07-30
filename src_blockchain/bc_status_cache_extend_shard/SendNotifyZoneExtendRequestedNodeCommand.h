/*
 * SendNotifyZoneExtendRequestedNodeCommand.h
 *
 *  Created on: Jul 19, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_EXTEND_SHARD_SENDNOTIFYZONEEXTENDREQUESTEDNODECOMMAND_H_
#define BC_STATUS_CACHE_EXTEND_SHARD_SENDNOTIFYZONEEXTENDREQUESTEDNODECOMMAND_H_

#include "bc_p2p_cmd_node_consensus/AbstractConsensusNodeCommand.h"

namespace codablecash {

class TransactionTransferData;
class BlockchainController;
class MemoryPool;
class NotifyZoneExtendRequestedTransaction;

class SendNotifyZoneExtendRequestedNodeCommand: public AbstractConsensusNodeCommand {
public:
	SendNotifyZoneExtendRequestedNodeCommand(const SendNotifyZoneExtendRequestedNodeCommand& inst);
	SendNotifyZoneExtendRequestedNodeCommand();
	virtual ~SendNotifyZoneExtendRequestedNodeCommand();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* buff) const;
	virtual void fromBinary(ByteBuffer* buff);

	virtual ByteBuffer* getSignBinary() const;
	virtual IBlockObject* copyData() const noexcept;

	virtual SystemTimestamp getFirstTimestamp() const;

	void setTransactionTransferData(const TransactionTransferData* data) noexcept;

protected:
	virtual AbstractCommandResponse* executeAsNode(BlockchainNodeHandshake* nodeHandShake, CodablecashNodeInstance* inst, bool suspend) const;

private:
	bool processTransaction(uint16_t zoneSelf, MemoryPool* memPool, BlockchainController* ctrl) const;

private:
	TransactionTransferData* data;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_EXTEND_SHARD_SENDNOTIFYZONEEXTENDREQUESTEDNODECOMMAND_H_ */
