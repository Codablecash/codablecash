/*
 * NotifyZoneExtendRequestedTransaction.h
 *
 *  Created on: Jun 29, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_EXTEND_SHARD_NOTIFYZONEEXTENDREQUESTEDTRANSACTION_H_
#define BC_STATUS_CACHE_EXTEND_SHARD_NOTIFYZONEEXTENDREQUESTEDTRANSACTION_H_

#include "bc_trx/AbstractInterChainCommunicationTansaction.h"

namespace codablecash {

class BlockHeaderId;
class UtxoId;
class BlockHeaderCommandId;

class NotifyZoneExtendRequestedTransaction : public AbstractInterChainCommunicationTansaction {
public:
	NotifyZoneExtendRequestedTransaction(const NotifyZoneExtendRequestedTransaction& inst);
	NotifyZoneExtendRequestedTransaction();
	virtual ~NotifyZoneExtendRequestedTransaction();

	virtual uint8_t getType() const noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual void build();

	virtual IBlockObject* copyData() const noexcept;

	virtual BalanceUnit getFee() const noexcept;
	virtual BalanceUnit getFeeRate() const noexcept;

	virtual int getUtxoSize() const noexcept;
	virtual AbstractUtxo* getUtxo(int i) const noexcept;
	virtual int getUtxoReferenceSize() const noexcept;
	virtual AbstractUtxoReference* getUtxoReference(int i) const noexcept;

	virtual bool validateOnAccept(MemPoolTransaction *memTrx, IStatusCacheContext* context) const;
	virtual TrxValidationResult validateFinal(const BlockHeader* header, MemPoolTransaction *memTrx, IStatusCacheContext* context) const;

	virtual bool checkFilter(const ArrayList<BloomFilter1024> *filtersList) const;

	void setHeaderInfo(uint16_t zone, uint64_t height, const BlockHeaderId* headerId);
	void setNewShardZone(uint16_t newShardZone) noexcept;
	void setCommandId(const BlockHeaderCommandId* commandId);

	uint16_t getNewShardZone() const noexcept {
		return this->newShardZone;
	}

private:
	// header info
	uint16_t zone;
	uint64_t height;
	BlockHeaderId* headerId;

	// new shard info
	uint16_t newShardZone;

	// Header Command Info
	BlockHeaderCommandId* commandId;

	// build automatically
	UtxoId* utxoId;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_EXTEND_SHARD_NOTIFYZONEEXTENDREQUESTEDTRANSACTION_H_ */
