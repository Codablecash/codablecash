/*
 * NewShardZoneCommand.h
 *
 *  Created on: Jun 13, 2026
 *      Author: iizuka
 */

#ifndef BC_BLOCK_HEADER_COMMAND_NEWSHARDZONECOMMAND_H_
#define BC_BLOCK_HEADER_COMMAND_NEWSHARDZONECOMMAND_H_

#include "bc_block_header_command/AbstractBlockHeaderCommand.h"

namespace codablecash {

class BlockHeaderId;
class Block;

class NewShardZoneCommand : public AbstractBlockHeaderCommand {
public:
	NewShardZoneCommand(const NewShardZoneCommand& inst);
	NewShardZoneCommand();
	virtual ~NewShardZoneCommand();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	void setNewShardZone(uint16_t newShardZone) noexcept;
	void setRequestingZone(uint16_t requestingZone) noexcept;
	void setGenesisblock(const Block* block);

	virtual void onFinalize(const BlockHeader *header, BlockchainStatusCache* statusCache, CodablecashBlockchain* blockchain, ILockinManager *lockinManager, const CodablecashSystemParam* config);

private:
	uint16_t newShardZone;
	uint16_t requestingZone;

	Block* genesisBlock;

};

} /* namespace codablecash */

#endif /* BC_BLOCK_HEADER_COMMAND_NEWSHARDZONECOMMAND_H_ */
