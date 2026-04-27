/*
 * BlockGenerator.h
 *
 *  Created on: 2023/03/29
 *      Author: iizuka
 */

#ifndef BC_BLOCK_GENERATOR_BLOCKGENERATOR_H_
#define BC_BLOCK_GENERATOR_BLOCKGENERATOR_H_

#include <cstdint>

namespace alinous {
class SystemTimestamp;
class BigInteger;
}
using namespace alinous;

namespace codablecash {

class CodablecashSystemParam;
class BlockchainController;
class MemoryPool;
class MemPoolTransaction;
class Block;
class CentralProcessor;
class IStatusCacheContext;
class BlockHeader;
class MiningConfig;
class VotingBlockStatus;
class VoteCandidate;
class VotePart;
class BlockHeaderId;
class PoWNonce;
class ISystemLogger;
class VoteBlockTransaction;
class ReservedVotes;

class BlockGenerator {
public:
	BlockGenerator(uint16_t zone, CodablecashSystemParam* config, MemoryPool* memoryPool, BlockchainController* ctrl, const MiningConfig *miningConfig, ISystemLogger* logger);
	virtual ~BlockGenerator();

	virtual void nonceCalculated(uint64_t lastBlockHeight, const BlockHeaderId *lastBlockId, const PoWNonce *nonce);

	virtual Block* generateBlock(uint64_t lastBlockHeight, const BlockHeaderId *lastBlockId, MemPoolTransaction* memTrx);
	virtual bool continueMining() const;
	virtual BlockHeader* changeMiningTarget();

	virtual void setScheduledBlock(const Block* block);

	virtual BigInteger calcTargetDiff(uint64_t lastHeight, const BlockHeaderId *lastHeaderId, const SystemTimestamp* tm);

	void setCentralProcessor(CentralProcessor* processor) noexcept {
		this->processor = processor;
	}

	MemoryPool* getMemoryPool() const noexcept {
		return this->memoryPool;
	}
	void setMemoryPool(MemoryPool* mempool) noexcept {
		this->memoryPool = mempool;
	}
	virtual BlockHeader* getTopBlockHeader() const;

	BlockchainController* getBlockchainController() const noexcept {
		return this->ctrl;
	}


protected:
	void importTransactions2Block(MemPoolTransaction* memTrx, Block* block);
	void buildRewordBase(MemPoolTransaction* memTrx, IStatusCacheContext* context, Block* block);

	void removkeUnvotedTickets(MemPoolTransaction *memTrx, IStatusCacheContext *context, Block *block, VotingBlockStatus* status);
	void revokeUnvokedcandidate(MemPoolTransaction *memTrx, IStatusCacheContext *context, Block *block, VoteCandidate* candidate);

	void addRevokeMissVotedTicket(MemPoolTransaction *memTrx, IStatusCacheContext *context, Block *block, VoteCandidate *candidate, const VoteBlockTransaction* trx);
	void addRevokeMissedTicket(MemPoolTransaction *memTrx, IStatusCacheContext *context, Block *block, VoteCandidate *candidate);

	void importControlTransactions2Block(MemPoolTransaction* memTrx, Block* block, const BlockHeader* header, IStatusCacheContext* context);
	void importInterChainCommunicationTransactions2Block(MemPoolTransaction* memTrx, Block* block, const BlockHeader* header, IStatusCacheContext* context);
	void importBalanceTransactions2Block(MemPoolTransaction* memTrx, Block* block, const BlockHeader* header, IStatusCacheContext* context);
	void importSmartcontractTransactions2Block(MemPoolTransaction* memTrx, Block* block, const BlockHeader* header, IStatusCacheContext* context);

protected:
	ISystemLogger* logger;

	uint16_t zone;

	CodablecashSystemParam* config;
	MemoryPool* memoryPool;
	BlockchainController* ctrl;

	CentralProcessor* processor;
	const MiningConfig *miningConfig;
};

} /* namespace codablecash */

#endif /* BC_BLOCK_GENERATOR_BLOCKGENERATOR_H_ */
