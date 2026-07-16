/*
 * NewShardAdder.h
 *
 *  Created on: Jun 24, 2026
 *      Author: iizuka
 */

#ifndef BLOCKCHAIN_MULTI_SHARD_NEWSHARDADDER_H_
#define BLOCKCHAIN_MULTI_SHARD_NEWSHARDADDER_H_

#include "bc_block_generator/IBlockGenerationListner.h"
#include <cstdint>

namespace codablecash {

class NewShardAdder: public IBlockGenerationListner {
public:
	NewShardAdder(const NewShardAdder& inst);
	NewShardAdder();
	virtual ~NewShardAdder();

	void init(uint16_t newShardZone, uint16_t requestingZone);

	virtual void onBlockGenerated(Block* block, MemPoolTransaction *memTrx, BlockchainController* ctrl);
	virtual IBlockGenerationListner* copy() const noexcept;

	void setHeight(uint64_t height);

private:
	void __addCommand(Block* block, MemPoolTransaction *memTrx, BlockchainController* ctrl);

private:
	uint64_t height;

	uint16_t newShardZone;
	uint16_t requestingZone;

};

} /* namespace codablecash */

#endif /* BLOCKCHAIN_MULTI_SHARD_NEWSHARDADDER_H_ */
