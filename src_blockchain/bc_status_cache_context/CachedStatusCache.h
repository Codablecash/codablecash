/*
 * CachedStatusCache.h
 *
 *  Created on: 2023/04/26
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_CONTEXT_CACHEDSTATUSCACHE_H_
#define BC_STATUS_CACHE_CONTEXT_CACHEDSTATUSCACHE_H_

#include <cstdint>

#include "base/ArrayList.h"

#include "bc_finalizer/VoterEntry.h"

namespace alinous {
class ConcurrentGate;
class File;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class TransactionContextCache;
class UtxoCacheContext;
class VoterStatusMappedCacheContext;
class StatusCacheContext;
class VoterEntry;
class NodeIdentifier;
class VotingBlockStatus;
class BlockHeaderId;
class UtxoData;
class TransactionId;
class AbstractBlockchainTransaction;
class RemoteUtxoHeight;

class CachedStatusCache {
public:
	static const constexpr wchar_t* CACHED_STATUS_PREFIX{L"contextcache"};

	CachedStatusCache(uint64_t serial, const File* cacheRepoBaseDir, uint64_t height);
	virtual ~CachedStatusCache();

	void init();
	void importStatusContext(StatusCacheContext *context);

	uint64_t getTicketPrice() const noexcept {
		return ticketPrice;
	}

	UtxoData* getData(const UtxoId* utxoId) const;

	uint64_t getHeight() const noexcept {
		return height;
	}

	const VoterStatusMappedCacheContext* getVoterStatusCacheContext() const noexcept {
		return this->voterCache;
	}

	AbstractBlockchainTransaction* getTransaction(const TransactionId *trxId) const;

	int getRequestedNewShards() const noexcept {
		return this->requestedNewShards;
	}
	uint16_t getNumZones() const noexcept {
		return this->numZones;
	}

	ArrayList<RemoteUtxoHeight>* getRemoteUtxoList() const noexcept {
		return this->utxolist;
	}

private:
	void importOtherCache(const CachedStatusCache* previousCache);

	void importTransactions(TransactionContextCache* cache);
	void importUxtoCache(UtxoCacheContext* cache);
	void importVoterStatusCache(VoterStatusMappedCacheContext* cache);

private:
	File* baseDir;

	uint64_t height;

	uint64_t ticketPrice;

	TransactionContextCache* trxCache;
	UtxoCacheContext* utxoCache;
	VoterStatusMappedCacheContext* voterCache;

	// new shards history
	int requestedNewShards;
	uint16_t numZones;

	// remote utxo
	ArrayList<RemoteUtxoHeight>* utxolist;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_CONTEXT_CACHEDSTATUSCACHE_H_ */
