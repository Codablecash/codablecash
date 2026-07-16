/*
 * ZoneStatusCache.h
 *
 *  Created on: 2023/03/19
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_ZONESTATUSCACHE_H_
#define BC_STATUS_CACHE_ZONESTATUSCACHE_H_

#include <cstdint>

namespace alinous {
class File;
class UnicodeString;
class BigInteger;
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class HeadBlockDetector;
class StatusStore;
class CodablecashBlockchain;
class BlockHeader;
class BlockHead;
class BlockHeaderId;
class FinalizedDataCache;
class IStatusCacheContext;
class UtxoData;
class UtxoId;
class MemPoolTransaction;
class CodablecashSystemParam;
class ISystemLogger;
class CodablecashSystemParam;
class Block;
class LockinManager;
class BlockHeaderStoreManager;
class VoteManager;
class RemoteUtxoRepository;

class ZoneStatusCache {
public:
	static const constexpr wchar_t* DIR_NAME_BASE{L"zone"};
	static const constexpr wchar_t* STATUS_STORE_FILE_NAME{L"status.bin"};

	static const constexpr wchar_t* KEY_ZONE{L"zone"};
	static const constexpr wchar_t* KEY_HEADER_ONLY{L"headerOnly"};
	static const constexpr wchar_t* KEY_FINALIZED_HEIGHT{L"finalizedHeight"};
	static const constexpr wchar_t* KEY_FINALIZED_TICKET_PRICE{L"finalizedTicketPrice"};
	static const constexpr wchar_t* KEY_REQUESTED_SHARDS{L"requestedShards"};

	ZoneStatusCache(const File* baseDir, uint16_t zone, bool headerOnly, ISystemLogger* logger, const CodablecashSystemParam* config);
	ZoneStatusCache(const File* baseDir, uint16_t zone, ISystemLogger* logger, bool headerOnly, const CodablecashSystemParam* config);
	virtual ~ZoneStatusCache();

	void initBlank();

	void open();
	void close();

	void updateBlockStatus(MemPoolTransaction* memTrx, CodablecashBlockchain *chain, const CodablecashSystemParam* config);
	void finalizeUpdateCacheData(uint64_t finalizingHeight, const BlockHeaderId *headerId, CodablecashBlockchain* blockchain, IStatusCacheContext* context, const CodablecashSystemParam* config);
	void finalizeHeaderUpdateCacheData(uint64_t finalizingHeight, const BlockHeaderId *headerId, CodablecashBlockchain* blockchain);

	const BlockHead* getHead() const noexcept;
	const BlockHead* getSecondHead() const noexcept;
	BlockHeader* changeMiningTarget();

	BigInteger calcTargetDifficulty(const BlockHeader* header, BlockHeaderStoreManager* headerManager
			, uint64_t powBlockTimeMills, uint16_t powHashrateBlocks, const SystemTimestamp* end) const;

	uint64_t getFinalizedHeight() const noexcept {
		return this->finalizedHeight;
	}
	uint64_t getTicketPrice() const noexcept {
		return this->ticketPrice;
	}

	FinalizedDataCache* getFinalizedDataCache() const noexcept {
		return this->finalizedCache;
	}

	LockinManager *getLockinManager() const noexcept {
		return this->lockinManager;
	}

	void setScheduledBlock(const Block* block);
	Block* fetchScheduledBlock();
	Block* getScheduledBlock();

	UtxoData* findUtxo(const UtxoId* utxoId) const;

	bool registerBlockHeader4Limit(const BlockHeader* header, const CodablecashSystemParam* param);
	SystemTimestamp* getPosVoteLimit(uint64_t lastHeight);

	int getRequestedNewShards() const noexcept {
		return this->requestedNewShards;
	}

	RemoteUtxoRepository* getRemoteUtxoRepository() const noexcept;

private:
	void addIdex2String(UnicodeString *str) const noexcept;

	void saveStatus();
	void loadStatus();

private:
	uint16_t zone;
	bool headerOnly;
	uint64_t finalizedHeight;

	uint64_t ticketPrice;

	File* baseDir;
	StatusStore* statusStore;
	HeadBlockDetector* headBlockDetector;

	FinalizedDataCache* finalizedCache;
	LockinManager* lockinManager;
	VoteManager* voteManager;

	int requestedNewShards;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_ZONESTATUSCACHE_H_ */
