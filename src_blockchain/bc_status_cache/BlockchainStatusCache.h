/*
 * BlockchainStatusCache.h
 *
 *  Created on: 2023/03/19
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_BLOCKCHAINSTATUSCACHE_H_
#define BC_STATUS_CACHE_BLOCKCHAINSTATUSCACHE_H_

#include "base/ArrayList.h"

#include <cstdint>

#include "bc_blockstore/IBlockchainEventListner.h"

namespace alinous {
class File;
class BigInteger;
class SysMutex;
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class ZoneStatusCache;
class StatusStore;
class PoWManager;
class CodablecashSystemParam;
class BlockHeader;
class MemPoolTransaction;
class BlockHeaderId;
class CodablecashBlockchain;
class BlockBody;
class BlockHead;
class FinalizerPool;
class IStatusCacheContext;
class UtxoData;
class UtxoId;
class MemoryPool;
class ISystemLogger;
class BlockHeaderStoreManager;
class LockinManager;


class BlockchainStatusCache : public IBlockchainEventListner {
public:
	static const constexpr wchar_t* CONFIG_FILE_NAME{L"cache.bin"};

	static const constexpr wchar_t* KEY_NUM_ZONES{L"numZones"};
	static const constexpr wchar_t* KEY_ZONE_SELF{L"zoneSelf"};

	explicit BlockchainStatusCache(const File* baseDir, const CodablecashSystemParam* config, MemoryPool* memPool, const File* tmpCacheBaseDir, ISystemLogger* logger);
	virtual ~BlockchainStatusCache();

	void initBlankCache(uint16_t zoneSelf, uint16_t numZones);

	void open();
	void close();
	void initCacheStatus(CodablecashBlockchain* blockchain);

	virtual void onBlockAdded(MemPoolTransaction* memTrx, const Block* block, CodablecashBlockchain* chain);
	virtual void postBlockAdded(const Block* block, CodablecashBlockchain* chain);
	virtual void onBlockHeaderAdded(MemPoolTransaction* memTrx, const BlockHeader* block, CodablecashBlockchain* chain);
//	virtual void postBlockHeaderAdded(const BlockHeader* block, CodablecashBlockchain* chain);

	void setPowManager(PoWManager* pow) noexcept;
	void report2PowManager(CodablecashBlockchain* blockchain, ZoneStatusCache* cache) noexcept;
	void reportFinalizing(uint16_t zone, const Block *block, CodablecashBlockchain *chain) const noexcept;

	void reportHeaderFinalizing(uint16_t zone, const BlockHeader *header, CodablecashBlockchain *chain) const noexcept;

	void setFinalizer(FinalizerPool* finalizer);

	void updateFinalizedCacheData(uint16_t zone, uint64_t finalizingHeight, const BlockHeaderId *headerId
			, CodablecashBlockchain* blockchain, MemPoolTransaction* memtrx, IStatusCacheContext* context);
	void updateFinalizedHeaderCacheData(uint16_t zone, uint64_t finalizingHeight, const BlockHeaderId *headerId
			, CodablecashBlockchain* blockchain, MemPoolTransaction* memTrx);

	uint64_t getHeadHeight(uint16_t zone);
	uint64_t getFinalizedHeight(uint16_t zone);
	const BlockHead* getHead(uint16_t zone) const noexcept;
	const BlockHead* getSecondHead(uint16_t zone) const noexcept;

	BlockHeader* changeMiningTarget(uint16_t zone);

	uint64_t getFinalizedTicketPrice(uint16_t zone);

	void importCosumedMemTransactions(uint16_t zone, MemPoolTransaction* memTrx, uint64_t heigh, const BlockHeaderId *headerId, CodablecashBlockchain* blockchain);

	uint16_t getNumZones(uint64_t height) const;

	ZoneStatusCache* getZoneStatusCache(uint16_t zone) const noexcept {
		return this->zoneList.get(zone);
	}

	UtxoData* findUtxo(uint16_t zone, const UtxoId* utxoId) const;

	void setMemoryPool(MemoryPool* memPool) noexcept {
		this->memPool = memPool;
	}

	BigInteger calcTargetDiff(uint16_t zone, uint64_t height, const BlockHeaderId* headerId, const SystemTimestamp* tm, BlockHeaderStoreManager* headerManager);
	BigInteger calcTargetDiff(BlockHeaderStoreManager* headerManager, ZoneStatusCache *cache, const BlockHeader* header, const SystemTimestamp* end);

	void setScheduledBlock(uint16_t zone, const Block *block);
	Block* fetchScheduledBlock(uint16_t zone);
	Block* getScheduledBlock(uint16_t zone);

	void requestPosVote(uint16_t zone, uint64_t calculatedNonceHeight, CodablecashBlockchain* blockchain);

	bool registerBlockHeader4Limit(uint16_t zone, const BlockHeader* header, const CodablecashSystemParam* param);
	SystemTimestamp* getPosVoteLimit(uint16_t zone, uint64_t lastHeight);

	LockinManager* getLockInManager(uint16_t zone) const noexcept;

private:
	void saveConfig();
	void loadConfig();

	void markConsumedTransactions(MemPoolTransaction *memTrx, BlockBody* body);

private:
	const CodablecashSystemParam* config;
	File* baseDir;

	ArrayList<ZoneStatusCache> zoneList;

	uint16_t numZones;
	uint16_t zoneSelf;
	StatusStore* statusStore;

	SysMutex* memberLock;
	PoWManager* powManager;
	FinalizerPool* finalizer;

	uint64_t lastVoted;

	MemoryPool* memPool;

	ISystemLogger* logger;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_BLOCKCHAINSTATUSCACHE_H_ */
