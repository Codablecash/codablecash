/*
 * GenerateNewGenesisBlockCommandMessage.h
 *
 *  Created on: Jul 5, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_EXTEND_SHARD_GENERATENEWGENESISBLOCKCOMMANDMESSAGE_H_
#define BC_STATUS_CACHE_EXTEND_SHARD_GENERATENEWGENESISBLOCKCOMMANDMESSAGE_H_

#include "bc_processor/AbstractCentralProcessorCommandMessage.h"
#include <cstdint>

namespace codablecash {

class Block;

class GenerateNewGenesisBlockCommandMessage : public AbstractCentralProcessorCommandMessage {
public:
	GenerateNewGenesisBlockCommandMessage();
	virtual ~GenerateNewGenesisBlockCommandMessage();

	void setNewShardZone(uint16_t newShardZone) noexcept;
	void setGenesisBlock(const Block* block);

protected:
	virtual void process(CentralProcessor* processor);

private:
	uint16_t newShardZone;
	Block* genesisBlock;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_EXTEND_SHARD_GENERATENEWGENESISBLOCKCOMMANDMESSAGE_H_ */
