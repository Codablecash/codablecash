/*
 * BlockchainController.h
 *
 *  Created on: 2023/03/21
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_BLOCKCHAINCONTROLLER_H_
#define BC_STATUS_CACHE_BLOCKCHAINCONTROLLER_H_

#include <cstdint>

#include "base/ArrayList.h"

namespace alinous {
class ConcurrentGate;
class File;
class StackWriteLock;
class BigInteger;
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class CodablecashBlockchain;
class BlockchainStatusCache;
class Block;
class BlockHeader;
class BlockHeaderId;
class MemPoolTransaction;
class IStatusCacheContext;
class StatusCacheContext;
class CodablecashSystemParam;
class CachedStatusCacheRepository;
class MemoryPool;
class ISystemLogger;
class IBlockDetectCallback;
class NodeIdentifier;
class VoterEntry;
class BloomFilter1024;
class AbstractBlockchainTransaction;


class BlockchainController {
public:
	BlockchainController(ISystemLogger* logger, const CodablecashSystemParam* config, const File* tmpCacheBaseDir, CodablecashBlockchain* blockchain, BlockchainStatusCache* statusCache, MemoryPool* memoryPool);
	virtual ~BlockchainController();

	bool addBlock(const Block* block);
	bool addBlockHeader(const BlockHeader* header);

	uint64_t getHeadHeight(uint16_t zone);
	uint16_t getZoneSelf() const noexcept;

	uint64_t getFinalizedHeight(uint16_t zone) const;

	void importCosumedMemTransactions(uint16_t zone, MemPoolTransaction* memTrx, uint64_t height, const BlockHeaderId *headerId);

	void requestMiningBlock(MemPoolTransaction* memTrx);
	void stopMining() noexcept;
	void stopFinalizing() noexcept;

	void finalize(uint16_t zone, uint64_t finalizingHeight, const BlockHeaderId *headerId, MemoryPool* memPool);
	void finalizeHeader(uint16_t zone, uint64_t finalizingHeight, const BlockHeaderId *headerId, MemoryPool* memPool);

	//BlockHeader* getNblocksBefore(uint16_t zone, const BlockHeaderId *headerId, uint64_t height, int voteBeforeNBlocks) const;
	BlockHeader* __getNblocksBefore(uint16_t zone, const BlockHeaderId *headerId, uint64_t height, int voteBeforeNBlocks) const;

	IStatusCacheContext* getStatusCacheContext(uint16_t zone, const BlockHeaderId* headerId, uint64_t height);
	IStatusCacheContext* getStatusCacheContext(uint16_t zone);
	IStatusCacheContext* getBlankStatusCacheContext(uint16_t zone);

	CodablecashBlockchain* getCodablecashBlockchain() const noexcept {
		return this->blockchain;
	}

	void getSyncHeaderData(uint16_t zone, uint64_t offsetHeight, int limit, IBlockDetectCallback* callback);

	int getMempoolTrxCount() const noexcept;
	ArrayList<AbstractBlockchainTransaction>* fetchMempoolTrx(const ArrayList<BloomFilter1024>* filters);

	static void checkFinalizedHeight(uint64_t finalizedHeight, uint64_t finalizingHeight);

	ArrayList<Block>* getBlocksHeightAt(uint16_t zone, uint64_t height) const;
	Block* getBlockHeightAt(uint16_t zone, uint64_t height, const BlockHeaderId* headerId) const;
	static Block* getBlockById(ArrayList<Block>* list, const BlockHeaderId* headerId);

	bool hasHeaderId(uint16_t zone, uint64_t height, const BlockHeaderId* headerId) const;

	BlockHeader* getTopHeader() const;
	BigInteger calcTargetDifficulty(uint16_t zone, uint64_t lastHeight, const BlockHeaderId *lastHeaderId, const SystemTimestamp* tm);
	bool checkAcceptSecondRealBlockOnMining() const;
	BlockHeader* changeMiningTarget();

	void setScheduledBlock(const Block *block);
	Block* fetechScheduledBlock();
	Block* __fetechScheduledBlock();
	Block* __getScheduledBlock();

	void requestPosVote(uint16_t zone, uint64_t calculatedNonceHeight);

	const VoterEntry* getVoterEntry(const NodeIdentifier* nodeId);

	bool registerBlockHeader4Limit(uint16_t zone, const BlockHeader* header, const CodablecashSystemParam* param);
	SystemTimestamp* getPosVoteLimit(uint16_t zone, uint64_t lastHeight);

	ISystemLogger* getLogger() const noexcept {
		return this->logger;
	}

private:
	IStatusCacheContext* doGetStatusCacheContext(uint16_t zone, const BlockHeaderId* headerId, uint64_t height, StackWriteLock* lock);
	void initCacehContext(uint16_t zone, StatusCacheContext* context, const BlockHeaderId *headerId, uint64_t height);

private:
	ISystemLogger* logger;

	File* tmpCacheBaseDir;

	MemoryPool* memoryPool;

	CodablecashBlockchain* blockchain;
	BlockchainStatusCache* statusCache;

	CachedStatusCacheRepository* statusCacheRepo;

	ConcurrentGate* rwLock;
	const CodablecashSystemParam* config;
};

class IBlockDetectCallback {
public:
	virtual ~IBlockDetectCallback(){};

	virtual void detect(uint16_t height, const ArrayList<Block>* list) = 0;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_BLOCKCHAINCONTROLLER_H_ */
