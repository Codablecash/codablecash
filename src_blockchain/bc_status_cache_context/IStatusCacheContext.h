/*
 * IStatusCacheContext.h
 *
 *  Created on: 2023/04/26
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_CONTEXT_ISTATUSCACHECONTEXT_H_
#define BC_STATUS_CACHE_CONTEXT_ISTATUSCACHECONTEXT_H_

#include "base/ArrayList.h"

#include "bc_finalizer/VoterEntry.h"

namespace alinous {
class File;
}
using namespace alinous;

namespace codablecash {

class BlockHeader;
class AbstractBalanceTransaction;
class AbstractControlTransaction;
class VoterEntry;
class VotingBlockStatus;
class BlockHeaderId;
class CodablecashSystemParam;
class CodablecashBlockchain;
class RegisterVotePoolTransaction;
class RegisterTicketTransaction;
class VoteBlockTransaction;
class AbstractUtxo;
class AbstractUtxoReference;
class AbstractBlockchainTransaction;
class AbstractInterChainCommunicationTansaction;
class AbstractSmartcontractTransaction;
class TransactionId;
class BlockchainStatusCache;
class ILockinManager;
class VoterStatusMappedCacheContext;
class ISystemLogger;
class BlockBody;
class RemoteUtxoDetector;

class IStatusCacheContext {
public:
	IStatusCacheContext();
	virtual ~IStatusCacheContext();

	virtual void close() = 0;

	virtual void importBalanceTransaction(const BlockHeader* header, const AbstractBalanceTransaction* trx, ISystemLogger* logger) = 0;
	virtual void importControlTransaction(const BlockHeader* header, const BlockBody *body, const AbstractControlTransaction* trx, ISystemLogger* logger) = 0;
	virtual void importInterChainCommunicationTransaction(const BlockHeader* header, const AbstractInterChainCommunicationTansaction* trx, ISystemLogger* logger) = 0;
	virtual void importSmartcontractTransaction(const BlockHeader* header, const AbstractSmartcontractTransaction* trx, ISystemLogger* logger) = 0;

	virtual ArrayList<VoterEntry, VoterEntry::VoteCompare>* getVoterEntries() const = 0;

	virtual VotingBlockStatus* getVotingBlockStatus(const BlockHeaderId *blockHeaderId) = 0;
	virtual VotingBlockStatus* getVotingBlockStatus(const BlockHeader *header) = 0;
	virtual const VoterEntry* getVoterEntry(const NodeIdentifier* nodeId) const noexcept = 0;

	virtual const CodablecashSystemParam* getConfig() const noexcept = 0;
	virtual CodablecashBlockchain* getBlockChain() const noexcept = 0;
	virtual BlockchainStatusCache* getBlockchainStatusCache() const noexcept = 0;
	virtual VoterStatusMappedCacheContext* getVoterStatusCacheContext() const noexcept = 0;

	virtual void loadInitialVotersData() = 0;

	virtual void registerVoterPool(const RegisterVotePoolTransaction* trx, uint64_t blockHeight, const BlockHeader *header, const BlockBody* body) = 0;
	virtual void registerTicket(const BlockHeader *header, const RegisterTicketTransaction* trx) = 0;
	virtual void registerVote(const BlockHeader *header, const VoteBlockTransaction* trx) = 0;

	virtual void beginBlock(const BlockHeader* header, ILockinManager* lockinManager, bool finalize) = 0;
	virtual void endBlock(const BlockHeader* header, ILockinManager* lockinManager, bool finalize) = 0;

	virtual uint16_t getZone() const noexcept = 0;
	virtual uint64_t getTicketPrice() const noexcept = 0;

	virtual AbstractUtxo* getUtxo(const UtxoId* utxoId) const = 0;
	virtual void removeUtxo(const AbstractUtxoReference *utxoref, const AbstractBlockchainTransaction *trx) = 0;

	virtual uint64_t getTopHeight() const noexcept = 0;

	virtual AbstractBlockchainTransaction* getTransaction(const TransactionId *trxId) = 0;

	virtual void setNumZones(uint16_t nuMzones) noexcept = 0;
	virtual void setRequestedNewShards(int requestedNewShards) noexcept = 0;
	virtual uint16_t getNumZones() const noexcept = 0;
	virtual int getRequestedNewShards() const noexcept = 0;

	virtual RemoteUtxoDetector* getRemoteUtxoDetector() const noexcept = 0;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_CONTEXT_ISTATUSCACHECONTEXT_H_ */
