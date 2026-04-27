/*
 * VoteBlockTransaction.cpp
 *
 *  Created on: 2023/04/15
 *      Author: iizuka
 */

#include "bc_finalizer_trx/VoteBlockTransaction.h"
#include "bc_finalizer_trx/TicketUtxoReference.h"
#include "bc_finalizer_trx/TicketVotedUtxo.h"

#include "bc_network/NodeIdentifier.h"
#include "bc_network/NodeIdentifierSource.h"

#include "base/StackRelease.h"
#include "bc/CodablecashSystemParam.h"

#include "crypto/Sha256.h"
#include "crypto/SchnorrSignature.h"

#include "bc_trx/TransactionId.h"
#include "bc_trx/AbstractUtxoReference.h"

#include "bc_base/BinaryUtils.h"

#include "bc_block/BlockHeaderId.h"
#include "bc_block/BlockHeader.h"

#include "crypto/Schnorr.h"

#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_blockstore_body/BlockBodyStoreManager.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_status_cache_context_finalizer/VotingBlockStatus.h"

namespace codablecash {

VoteBlockTransaction::VoteBlockTransaction(const VoteBlockTransaction &inst)
		: AbstractFinalizerTransaction(inst) {
	this->voterId = inst.voterId != nullptr ? new NodeIdentifier(*inst.voterId) : nullptr;
	this->ticketUtxoRef = inst.ticketUtxoRef != nullptr ? dynamic_cast<TicketUtxoReference*>(inst.ticketUtxoRef->copyData()) : nullptr;
	this->votedUtxo = inst.votedUtxo != nullptr ? dynamic_cast<TicketVotedUtxo*>(inst.votedUtxo->copyData()) : nullptr;

	this->voteBlockHeaderId = inst.voteBlockHeaderId != nullptr ? new BlockHeaderId(*inst.voteBlockHeaderId) : nullptr;
	this->voteBlockHeight = inst.voteBlockHeight;
	this->sig = new SchnorrSignature(*inst.sig);
}

VoteBlockTransaction::VoteBlockTransaction() : AbstractFinalizerTransaction() {
	this->voterId = nullptr;
	this->ticketUtxoRef = nullptr;
	this->votedUtxo = nullptr;

	this->voteBlockHeaderId = nullptr;
	this->voteBlockHeight = 0;
	this->sig = nullptr;
}

VoteBlockTransaction::~VoteBlockTransaction() {
	delete this->voterId;
	delete this->ticketUtxoRef;
	delete this->votedUtxo;

	delete this->voteBlockHeaderId;
	delete this->sig;
}

uint8_t VoteBlockTransaction::getType() const noexcept {
	return AbstractFinalizerTransaction::TRX_TYPE_VOTE_BLOCK;
}

bool VoteBlockTransaction::validateOnAccept(MemPoolTransaction *memTrx, IStatusCacheContext *context) const {
	uint64_t topHeight = context->getTopHeight();

	// check if block exists
	if(topHeight >= this->voteBlockHeight){
		uint16_t zone = context->getZone();
		CodablecashBlockchain* chain = context->getBlockChain();
		BlockHeaderStoreManager* headerManager = chain->getHeaderManager(zone);

		BlockHeader* vheader = headerManager->getHeader(this->voteBlockHeaderId, this->voteBlockHeight); __STP(vheader);
		if(vheader == nullptr){
			return false;
		}
	}

	UtxoValidationResult utxoRes = validateUtxos(memTrx, context, this->fee);
	if(utxoRes == UtxoValidationResult::INVALID){
		return false;
	}

	return verify();
}

TrxValidationResult VoteBlockTransaction::validateFinal(const BlockHeader *header, MemPoolTransaction *memTrx, IStatusCacheContext *context) const {
	return __validateFinal(header, memTrx, context, true);
}



TrxValidationResult VoteBlockTransaction::validateReported(const BlockHeader *header, IStatusCacheContext *context) const {
	return __validateFinal(header, nullptr, context, false);
}

TrxValidationResult VoteBlockTransaction::__validateFinal(const BlockHeader* header, MemPoolTransaction *memTrx, IStatusCacheContext* context, bool checkVotedHeaderId) const {
	UtxoValidationResult utxoRes = validateUtxos(memTrx, context, this->fee);

	if(utxoRes == UtxoValidationResult::INVALID){
		return TrxValidationResult::INVALID;
	}

	const CodablecashSystemParam* config = context->getConfig();
	uint64_t blockHeight = header->getHeight(); // this block
	uint16_t zone = header->getZone();

	// config
	uint16_t voteBeforeNBlocks = config->getVoteBeforeNBlocks(blockHeight);
	uint16_t voteBlockIncludeAfterNBlocks = config->getVoteBlockIncludeAfterNBlocks(blockHeight);

	uint64_t voteTargetBlockHeight = this->voteBlockHeight + voteBeforeNBlocks; // voting block height
	uint64_t includeBlockHeight = voteTargetBlockHeight + voteBlockIncludeAfterNBlocks; // block height to include

	CodablecashBlockchain* chain = context->getBlockChain();
	BlockHeaderStoreManager* headerManager = chain->getHeaderManager(zone);

	// check voting status
	bool isSelected = false;
	{
		const UtxoId* ticketUtxoId = this->ticketUtxoRef->getUtxoId();
		VotingBlockStatus* status = context->getVotingBlockStatus(header); __STP(status);

		isSelected = (status != nullptr && status->hasVoteCandidate(ticketUtxoId));
	}

	// check if block exists
	if(checkVotedHeaderId){
		BlockHeader* vheader = headerManager->getHeader(this->voteBlockHeaderId, this->voteBlockHeight); __STP(vheader);
		if(vheader == nullptr){
			return TrxValidationResult::INVALID;
		}
	}

	if(isSelected && blockHeight == includeBlockHeight){
		return TrxValidationResult::OK;
	}
	else if(blockHeight > includeBlockHeight){
		return TrxValidationResult::INVALID;
	}

	return TrxValidationResult::PENDING;
}

int VoteBlockTransaction::binarySize() const {
	BinaryUtils::checkNotNull(this->voterId);
	BinaryUtils::checkNotNull(this->ticketUtxoRef);
	BinaryUtils::checkNotNull(this->votedUtxo);
	BinaryUtils::checkNotNull(this->voteBlockHeaderId);

	int total = AbstractControlTransaction::__binarySize();

	total += this->voterId->binarySize();
	total += this->ticketUtxoRef->binarySize();
	total += this->votedUtxo->binarySize();
	total += this->voteBlockHeaderId->binarySize();
	total += sizeof(this->voteBlockHeight);

	total += this->sig->binarySize();

	return total;
}

void VoteBlockTransaction::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->voterId);
	BinaryUtils::checkNotNull(this->ticketUtxoRef);
	BinaryUtils::checkNotNull(this->votedUtxo);
	BinaryUtils::checkNotNull(this->voteBlockHeaderId);

	AbstractControlTransaction::__toBinary(out);

	this->voterId->toBinary(out);
	this->ticketUtxoRef->toBinary(out);
	this->votedUtxo->toBinary(out);
	this->voteBlockHeaderId->toBinary(out);
	out->putLong(this->voteBlockHeight);

	this->sig->toBinary(out);
}

void VoteBlockTransaction::fromBinary(ByteBuffer *in) {
	AbstractControlTransaction::__fromBinary(in);

	this->voterId = NodeIdentifier::fromBinary(in);

	AbstractUtxoReference* ref = AbstractUtxoReference::createFromBinary(in); __STP(ref);
	this->ticketUtxoRef = dynamic_cast<TicketUtxoReference*>(ref);
	BinaryUtils::checkNotNull(this->ticketUtxoRef);
	__STP_MV(ref);

	AbstractUtxo* vutxo = AbstractUtxo::createFromBinary(in); __STP(vutxo);
	this->votedUtxo = dynamic_cast<TicketVotedUtxo*>(vutxo);
	BinaryUtils::checkNotNull(this->votedUtxo);
	__STP_MV(vutxo);

	this->voteBlockHeaderId = BlockHeaderId::fromBinary(in);
	this->voteBlockHeight = in->getLong();

	this->sig = SchnorrSignature::createFromBinary(in);
}

void VoteBlockTransaction::build() {
	setUtxoNonce();

	int capacity = __binarySize();
	capacity += this->voterId->binarySize();
	capacity += this->ticketUtxoRef->binarySize();
	capacity += this->votedUtxo->binarySize();
	capacity += this->voteBlockHeaderId->binarySize();
	capacity += sizeof(this->voteBlockHeight);

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);

	__toBinary(buff);
	this->voterId->toBinary(buff);
	this->ticketUtxoRef->toBinary(buff);
	this->votedUtxo->toBinary(buff);
	this->voteBlockHeaderId->toBinary(buff);
	buff->putLong(this->voteBlockHeight);
	__ASSERT_POS(buff);

	buff->position(0);

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);

	delete this->trxId;
	this->trxId = new TransactionId((const char*)sha->array(), sha->limit());
}


IBlockObject* VoteBlockTransaction::copyData() const noexcept {
	return new VoteBlockTransaction(*this);
}

int VoteBlockTransaction::getUtxoReferenceSize() const noexcept {
	return AbstractFinalizerTransaction::getUtxoReferenceSize() + 1;
}

AbstractUtxoReference* VoteBlockTransaction::getUtxoReference(int i) const noexcept {
	int cap = AbstractFinalizerTransaction::getUtxoReferenceSize();

	return i < cap ? AbstractFinalizerTransaction::getUtxoReference(i) : this->ticketUtxoRef;
}

int VoteBlockTransaction::getUtxoSize() const noexcept {
	return AbstractFinalizerTransaction::getUtxoSize() + 1;
}

codablecash::AbstractUtxo* VoteBlockTransaction::getUtxo(int i) const noexcept {
	int cap = AbstractFinalizerTransaction::getUtxoSize();

	return i < cap ? AbstractFinalizerTransaction::getUtxo(i) : this->votedUtxo;
}

void VoteBlockTransaction::setVoterId(const NodeIdentifier *voterId) noexcept {
	delete this->voterId;
	this->voterId = new NodeIdentifier(*voterId);
}

void VoteBlockTransaction::setTicketUtxoId(const UtxoId *utxoId, const BalanceUnit amount, const AddressDescriptor* desc) noexcept {
	delete this->ticketUtxoRef;

	this->ticketUtxoRef = new TicketUtxoReference();
	this->ticketUtxoRef->setUtxoId(utxoId, desc);

	delete this->votedUtxo;
	this->votedUtxo = new TicketVotedUtxo();
	this->votedUtxo->setAddress(desc);
	this->votedUtxo->setVotedUtxoId(utxoId);
	this->votedUtxo->setAmount(amount);
}

void VoteBlockTransaction::setVoteBlockId(const BlockHeaderId *voteBlockHeaderId) noexcept {
	delete this->voteBlockHeaderId;

	this->voteBlockHeaderId = new BlockHeaderId(*voteBlockHeaderId);
}

void VoteBlockTransaction::sign(const NodeIdentifierSource *source) noexcept {
	delete this->sig;

	const char* data = this->trxId->toArray();
	int length = this->trxId->size();

	this->sig = Schnorr::sign(*source->getSecretKey(), *this->voterId->getPublicKey(), (const uint8_t*)data, (size_t)length);
}

bool VoteBlockTransaction::verify() const noexcept {
	const char* data = this->trxId->toArray();
	int length = this->trxId->size();

	bool result = Schnorr::verify(*this->sig, *this->voterId->getPublicKey(), (const uint8_t*)data, (size_t)length);
	return result;
}

void VoteBlockTransaction::setFeeAmount(const BalanceUnit *fee) {
	this->fee = *fee;
}

} /* namespace codablecash */
