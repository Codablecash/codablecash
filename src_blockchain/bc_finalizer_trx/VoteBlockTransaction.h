/*
 * VoteBlockTransaction.h
 *
 *  Created on: 2023/04/15
 *      Author: iizuka
 */

#ifndef BC_FINALIZER_TRX_VOTEBLOCKTRANSACTION_H_
#define BC_FINALIZER_TRX_VOTEBLOCKTRANSACTION_H_

#include "bc_finalizer_trx/AbstractFinalizerTransaction.h"

namespace codablecash {

class NodeIdentifier;
class TicketUtxoReference;
class UtxoId;
class BlockHeaderId;
class SchnorrSignature;
class NodeIdentifierSource;
class TicketVotedUtxo;
class AddressDescriptor;

class VoteBlockTransaction : public AbstractFinalizerTransaction {
public:
	VoteBlockTransaction(const VoteBlockTransaction& inst);
	VoteBlockTransaction();
	virtual ~VoteBlockTransaction();

	virtual uint8_t getType() const noexcept;

	virtual bool validateOnAccept(MemPoolTransaction *memTrx, IStatusCacheContext* context) const;
	virtual TrxValidationResult validateFinal(const BlockHeader* header, MemPoolTransaction *memTrx, IStatusCacheContext* context) const;
	virtual TrxValidationResult validateReported(const BlockHeader* header, IStatusCacheContext* context) const;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;

	virtual void fromBinary(ByteBuffer* in);

	virtual void build();
	virtual IBlockObject* copyData() const noexcept;

	virtual int getUtxoReferenceSize() const noexcept;
	virtual AbstractUtxoReference* getUtxoReference(int i) const noexcept;
	virtual int getUtxoSize() const noexcept;
	virtual AbstractUtxo* getUtxo(int i) const noexcept;

	void setZone(uint16_t z) noexcept;
	/*uint16_t getZone() const noexcept {
		return this->zone;
	}*/

	void setVoterId(const NodeIdentifier* voterId) noexcept;
	void setTicketUtxoId(const UtxoId* utxoId, const BalanceUnit amount, const AddressDescriptor* desc) noexcept;

	void setVoteBlockId(const BlockHeaderId* voteBlockHeaderId) noexcept;
	void setVoteBlockHeight(uint64_t voteBlockHeight) noexcept {
		this->voteBlockHeight = voteBlockHeight;
	}

	void sign(const NodeIdentifierSource* source) noexcept;
	bool verify() const noexcept;

	BlockHeaderId* getVotedHeaderId() const noexcept {
		return this->voteBlockHeaderId;
	}
	uint64_t getVotedHeaderBlockHeight() const noexcept {
		return this->voteBlockHeight;
	}

	const TicketUtxoReference* getTicketUtxoReference() const noexcept {
		return this->ticketUtxoRef;
	}

	const TicketVotedUtxo* getTicketVotedUtxo() const noexcept {
		return this->votedUtxo;
	}

	virtual void setFeeAmount(const BalanceUnit *fee);

private:
	TrxValidationResult __validateFinal(const BlockHeader* header, MemPoolTransaction *memTrx, IStatusCacheContext* context, bool checkVotedHeaderId) const;

private:
	uint16_t zone;
	NodeIdentifier* voterId;
	TicketUtxoReference* ticketUtxoRef;
	TicketVotedUtxo* votedUtxo;

	BlockHeaderId* voteBlockHeaderId; // voted block header
	uint64_t voteBlockHeight; // of voted block

	SchnorrSignature* sig;
};

} /* namespace codablecash */

#endif /* BC_FINALIZER_TRX_VOTEBLOCKTRANSACTION_H_ */
