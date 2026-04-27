/*
 * ClientBlockMinedCommand.h
 *
 *  Created on: Aug 18, 2025
 *      Author: iizuka
 */

#ifndef BC_WALLET_NET_CMD_CLIENTBLOCKMINEDCOMMAND_H_
#define BC_WALLET_NET_CMD_CLIENTBLOCKMINEDCOMMAND_H_

#include "bc_wallet_net_cmd/AbstractClientQueueCommand.h"

#include "data_history_data/BlockHeaderTransferData.h"


namespace codablecash {

class NodeIdentifier;
class BlockHeader;
class WalletNetworkManager;
class ISystemLogger;
class NetworkWalletData;

class ClientBlockMinedCommand : public AbstractClientQueueCommand {
public:
	ClientBlockMinedCommand(const ClientBlockMinedCommand& inst);
	ClientBlockMinedCommand();
	virtual ~ClientBlockMinedCommand();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	virtual void process(NetworkWallet* wallet) const;

	void setData(const BlockHeaderTransferData* data);

	void setSourceNodeId(const NodeIdentifier* nodeId);

private:
	BlockHeader* processBlock(uint16_t zone, const BlockHeaderId* headerId, uint64_t height, const NodeIdentifier* nodeId, WalletNetworkManager* networkManager, ISystemLogger* logger, NetworkWalletData* walletData) const;
	void checkOrphan(const BlockHeader* header, NetworkWallet *wallet, WalletNetworkManager *networkManager, ISystemLogger *logger) const;

private:
	BlockHeaderTransferData* data;
	NodeIdentifier* sourceNodeId;
};

} /* namespace codablecash */

#endif /* BC_WALLET_NET_CMD_CLIENTBLOCKMINEDCOMMAND_H_ */
