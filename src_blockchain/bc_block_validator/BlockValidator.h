/*
 * BlockValidator.h
 *
 *  Created on: 2023/11/27
 *      Author: iizuka
 */

#ifndef BC_BLOCK_VALIDATOR_BLOCKVALIDATOR_H_
#define BC_BLOCK_VALIDATOR_BLOCKVALIDATOR_H_

namespace codablecash {

class Block;
class CodablecashSystemParam;
class MemoryPool;
class BlockchainController;
class MemPoolTransaction;
class BlockHeader;
class IStatusCacheContext;
class MiningConfig;
class VotingBlockStatus;
class VoteCandidate;
class BlockHeaderId;
class BlockBody;

class BlockValidator {
public:
	BlockValidator(const Block* block, CodablecashSystemParam* config, MemoryPool* memoryPool, BlockchainController* ctrl);
	virtual ~BlockValidator();

	void validate();
	void validateHeaderCommand(); // if mined self

private:
	void validateLastHeader();
	void validateHashrate();

	void validateTransactionsInBlock(MemPoolTransaction *memTrx);
	void validateControlTransactions(MemPoolTransaction* memTrx, const BlockHeader* header, IStatusCacheContext* context);
	void validateInterChainCommunicationTransactions(MemPoolTransaction* memTrx, const BlockHeader* header, IStatusCacheContext* context);
	void validateBalanceTransactions(MemPoolTransaction* memTrx, const BlockHeader* header, IStatusCacheContext* context);
	void validateSmartcontractTransactions(MemPoolTransaction* memTrx, const BlockHeader* header, IStatusCacheContext* context);

	void validateRewordBase(MemPoolTransaction *memTrx, IStatusCacheContext *context);
	void validateUnvotedTickets(MemPoolTransaction *memTrx, IStatusCacheContext *context, VotingBlockStatus* status);
	void validateUnvokedcandidate(MemPoolTransaction *memTrx, IStatusCacheContext *context, VoteCandidate *candidate, const BlockHeaderId* correctVotedHeaderId);

private:
	const Block* block;

	CodablecashSystemParam* config;
	MemoryPool* memoryPool;
	BlockchainController* ctrl;
};

} /* namespace codablecash */

#endif /* BC_BLOCK_VALIDATOR_BLOCKVALIDATOR_H_ */
