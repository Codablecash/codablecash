/*
 * StatusCacheContext.h
 *
 *  Created on: 2023/04/23
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_CONTEXT_STATUSCACHECONTEXT_H_
#define BC_STATUS_CACHE_CONTEXT_STATUSCACHECONTEXT_H_

#include <cstdint>

#include "base/ArrayList.h"

#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc_finalizer/VoterEntry.h"
namespace alinous {
class ConcurrentGate;
class File;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class BlockchainStatusCache;
class BlockHeader;
class BlockBody;
class TransactionContextCache;
class UtxoCacheContext;
class AbstractBlockchainTransaction;
class VoterStatusCacheContext;
class RegisterVotePoolTransaction;
class CodablecashSystemParam;
class AbstractBalanceTransaction;
class AbstractControlTransaction;
class VoterEntry;
class RegisterTicketTransaction;
class NodeIdentifier;
class CodablecashBlockchain;
class VoteBlockTransaction;
class AbstractUtxo;
class AbstractUtxoReference;
class CachedStatusCache;
class ReservedVotesStore;
class TransactionId;
class RemoteUtxoDetector;

class StatusCacheContext : public IStatusCacheContext {
public:
	static const constexpr wchar_t* CONTEXT_CACHE_PREFIX{L"contextcache"};
	static uint64_t getSerial() noexcept;

	StatusCacheContext(const CodablecashSystemParam* config, const File* tmpCacheBaseDir, uint16_t zone,
			ConcurrentGate* rwLock, BlockchainStatusCache* statusCache, CodablecashBlockchain* blockchain,
			const wchar_t* prefix=CONTEXT_CACHE_PREFIX);
	virtual ~StatusCacheContext();

	void init();
	virtual void close();

	void importBlock(const BlockHeader* header, const BlockBody* blockBody, ISystemLogger* logger);

	virtual void beginBlock(const BlockHeader* header, ILockinManager* lockinManager, bool finalize);
	virtual void endBlock(const BlockHeader* header, ILockinManager* lockinManager, bool finalize);

	virtual void importBalanceTransaction(const BlockHeader* header, const AbstractBalanceTransaction* trx, ISystemLogger* logger);
	virtual void importControlTransaction(const BlockHeader* header, const BlockBody* body, const AbstractControlTransaction* trx, ISystemLogger* logger);
	virtual void importInterChainCommunicationTransaction(const BlockHeader* header, const AbstractInterChainCommunicationTansaction* trx, ISystemLogger* logger);
	virtual void importSmartcontractTransaction(const BlockHeader* header, const AbstractSmartcontractTransaction* trx, ISystemLogger* logger);

	virtual ArrayList<VoterEntry, VoterEntry::VoteCompare>* getVoterEntries() const;

	virtual VotingBlockStatus* getVotingBlockStatus(const BlockHeaderId *blockHeaderId);

	TransactionContextCache* getTransactionContextCache() const noexcept {
		return this->trxCache;
	}
	UtxoCacheContext* getUtxoCacheContext() const noexcept {
		return this->utxoCache;
	}
	virtual VoterStatusMappedCacheContext* getVoterStatusCacheContext() const noexcept {
		return this->voterCache;
	}

	virtual const VoterEntry* getVoterEntry(const NodeIdentifier* nodeId) const noexcept;

	virtual const CodablecashSystemParam* getConfig() const noexcept {
		return this->config;
	}
	virtual CodablecashBlockchain* getBlockChain() const noexcept {
		return this->blockchain;
	}
	virtual BlockchainStatusCache* getBlockchainStatusCache() const noexcept {
		return this->statusCache;
	}

	virtual void loadInitialVotersData();

	virtual void registerVoterPool(const RegisterVotePoolTransaction* trx, uint64_t blockHeight, const BlockHeader *header, const BlockBody* body);
	virtual void registerTicket(const BlockHeader *header, const RegisterTicketTransaction* trx);
	virtual void registerVote(const BlockHeader *header, const VoteBlockTransaction* trx);

	virtual uint16_t getZone() const noexcept {
		return this->zone;
	}

	virtual uint64_t getTicketPrice() const noexcept {
		return this->ticketPrice;
	}

	virtual AbstractUtxo* getUtxo(const UtxoId* utxoId) const;

	virtual const CachedStatusCache* getCachedStatusCache() const noexcept {
		return nullptr;
	}
	virtual uint64_t getPreAnalyzedHeight() const noexcept;


	virtual uint64_t getTopHeight() const noexcept {
		return this->topHeight;
	}
	void setTopHeight(uint64_t height) noexcept {
		this->topHeight = height;
	}

	virtual AbstractBlockchainTransaction* getTransaction(const TransactionId *trxId);

	virtual void setNumZones(uint16_t numZones) noexcept;
	virtual void setRequestedNewShards(int requestedNewShards) noexcept;
	virtual uint16_t getNumZones() const noexcept {
		return this->numZones;
	}
	virtual int getRequestedNewShards() const noexcept {
		return this->requestedNewShards;
	}

	virtual RemoteUtxoDetector* getRemoteUtxoDetector() const noexcept {
		return this->remoteUtxos;
	}

protected:
	void handleHeaderCommands(const BlockHeader *header, ILockinManager* lockinManager);

	void importBalanceTransactions(const BlockHeader* header, const BlockBody* blockBody, ISystemLogger* logger);
	void importControlTransactions(const BlockHeader* header, const BlockBody* blockBody, ISystemLogger* logger);
	void importInterChainCommunicationTransactions(const BlockHeader* header, const BlockBody* blockBody, ISystemLogger* logger);
	void importSmartcontractTransactions(const BlockHeader* header, const BlockBody* blockBody, ISystemLogger* logger);

	void importRewordTransactions(const BlockHeader* header, const BlockBody* blockBody, ISystemLogger* logger);

	void importUtxo(const AbstractBlockchainTransaction* trx, const BlockHeader *header);
	virtual void putUtxo(const AbstractUtxo* utxo, const AbstractBlockchainTransaction *trx, const BlockHeader *header);
	virtual void removeUtxo(const AbstractUtxoReference *utxoref, const AbstractBlockchainTransaction *trx);

	virtual VotingBlockStatus* getVotingBlockStatus(const BlockHeader *header);

	void calcTicketPrice(const BlockHeader *header);

protected:
	ConcurrentGate* rwLock;
	BlockchainStatusCache* statusCache;
	CodablecashBlockchain* blockchain;
	uint16_t zone;

	uint64_t ticketPrice;

	File* baseDir;

	TransactionContextCache* trxCache;
	UtxoCacheContext* utxoCache;
	/**
	 * this cache must includes cached data.
	 * by calling loadInitialVotersData();
	 */
	VoterStatusMappedCacheContext* voterCache;

	const CodablecashSystemParam* config;

	uint64_t topHeight;

	// new shards history
	int requestedNewShards;
	uint16_t numZones;

	// remote utxo
	RemoteUtxoDetector* remoteUtxos;

};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_CONTEXT_STATUSCACHECONTEXT_H_ */
