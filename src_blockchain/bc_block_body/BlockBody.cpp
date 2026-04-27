/*
 * BlockBody.cpp
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#include "bc_block_body/BlockBody.h"
#include "bc_block_body/BlockRewordBase.h"
#include "bc_block_body/OmittedBlockBody.h"
#include "bc_block_body/ITransactionVisitor.h"

#include "bc_trx/AbstractBalanceTransaction.h"
#include "bc_trx/AbstractControlTransaction.h"
#include "bc_trx/TransactionId.h"
#include "bc_trx/UtxoId.h"
#include "bc_trx/AbstractInterChainCommunicationTansaction.h"

#include "bc_finalizer_trx/VoteBlockTransaction.h"
#include "bc_finalizer_trx/TicketUtxoReference.h"
#include "bc_finalizer_trx/RevokeMissVotedTicket.h"
#include "bc_finalizer_trx/TicketVotedUtxoReference.h"
#include "bc_finalizer_trx/RevokeMissedTicket.h"

#include "merkletree/AbstractMerkleElement.h"
#include "merkletree/MerkleTree.h"

#include "base/StackRelease.h"

#include "bc_block/BlockMerkleRoot.h"

#include "bc_base/BinaryUtils.h"

#include "osenv/funcs.h"

#include "transaction/AbstractSmartcontractTransaction.h"


namespace codablecash {

BlockBody::BlockBody() {
	this->nonce = 0L;

	this->tree = nullptr;
	this->merkleRoot = nullptr;
	this->balanceTransactions = new ArrayList<AbstractBalanceTransaction>();
	this->controlTransactions = new ArrayList<AbstractControlTransaction>();
	this->iccTransactions = new ArrayList<AbstractInterChainCommunicationTansaction>();
	this->smartcontractTransactions = new ArrayList<AbstractSmartcontractTransaction>();
	this->rewardBase = new BlockRewordBase();
}

BlockBody::BlockBody(uint64_t nonce) {
	this->nonce = nonce;

	this->tree = nullptr;
	this->merkleRoot = nullptr;
	this->balanceTransactions = new ArrayList<AbstractBalanceTransaction>();
	this->controlTransactions = new ArrayList<AbstractControlTransaction>();
	this->iccTransactions = new ArrayList<AbstractInterChainCommunicationTansaction>();
	this->smartcontractTransactions = new ArrayList<AbstractSmartcontractTransaction>();
	this->rewardBase = new BlockRewordBase();
}

BlockBody::~BlockBody() {
	resetMerkle();

	this->smartcontractTransactions->deleteElements();
	delete this->smartcontractTransactions;

	this->balanceTransactions->deleteElements();
	delete this->balanceTransactions;

	this->controlTransactions->deleteElements();
	delete this->controlTransactions;

	this->iccTransactions->deleteElements();
	delete this->iccTransactions;

	delete this->rewardBase;
}

void BlockBody::addBalanceTransaction(	const AbstractBalanceTransaction *trx) noexcept {
	this->balanceTransactions->addElement(dynamic_cast<AbstractBalanceTransaction*>(trx->copyData()));
}

void BlockBody::addControlTransaction(	const AbstractControlTransaction *trx) noexcept {
	this->controlTransactions->addElement(dynamic_cast<AbstractControlTransaction*>(trx->copyData()));
}

void BlockBody::addInterChainCommunicationTransaction(const AbstractInterChainCommunicationTansaction *trx) noexcept {
	this->iccTransactions->addElement(dynamic_cast<AbstractInterChainCommunicationTansaction*>(trx->copyData()));
}

void BlockBody::addSmartcontractTransaction(const AbstractSmartcontractTransaction *trx) noexcept {
	this->smartcontractTransactions->addElement(dynamic_cast<AbstractSmartcontractTransaction*>(trx->copyData()));
}

void BlockBody::addBlockRewordTransaction(const AbstractBlockRewordTransaction *trx) noexcept {
	this->rewardBase->addBlockRewordTransaction(trx);
}

void BlockBody::build() {
	resetMerkle();

	this->tree = new MerkleTree();
	this->tree->addElement(this->nonce);

	{
		int maxLoop = this->controlTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractControlTransaction* trx = this->controlTransactions->get(i);
			this->tree->addElement(trx);
			trx->addInternalMerkleTreeElement(this->tree);
		}
	}
	{
		int maxLoop = this->iccTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractInterChainCommunicationTansaction* trx = this->iccTransactions->get(i);
			this->tree->addElement(trx);
			trx->addInternalMerkleTreeElement(this->tree);
		}
	}
	{
		int maxLoop = this->balanceTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractBalanceTransaction* trx = this->balanceTransactions->get(i);
			this->tree->addElement(trx);
			trx->addInternalMerkleTreeElement(this->tree);
		}
	}
	{
		int maxLoop = this->smartcontractTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractSmartcontractTransaction* trx = this->smartcontractTransactions->get(i);
			this->tree->addElement(trx);
			trx->addInternalMerkleTreeElement(this->tree);
		}
	}

	this->rewardBase->buildMerkleTree(this->tree);

	this->tree->pack();
	const AbstractMerkleElement* element = this->tree->getRoot();

	const ByteBuffer* buff = element->getHash();
	this->merkleRoot = new BlockMerkleRoot((const char*)buff->array(), buff->limit());
}

int BlockBody::binarySize() const {
	BinaryUtils::checkNotNull(this->rewardBase);

	int total = sizeof(this->nonce);

	{
		int maxLoop = this->controlTransactions->size();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_CONTROL_TRX);
		total += sizeof(uint16_t);

		for(int i = 0; i != maxLoop; ++i){
			AbstractControlTransaction* trx = this->controlTransactions->get(i);
			total += trx->binarySize();
		}
	}

	{
		int maxLoop = this->iccTransactions->size();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_ICC_TRX);
		total += sizeof(uint16_t);

		for(int i = 0; i != maxLoop; ++i){
			AbstractInterChainCommunicationTansaction* trx = this->iccTransactions->get(i);
			total += trx->binarySize();
		}
	}

	{
		int maxLoop = this->balanceTransactions->size();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_BALANCE_TRX);
		total += sizeof(uint16_t);

		for(int i = 0; i != maxLoop; ++i){
			AbstractBalanceTransaction* trx = this->balanceTransactions->get(i);
			total += trx->binarySize();
		}
	}

	{
		int maxLoop = this->smartcontractTransactions->size();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_SMARTCONTRACT_TRX);
		total += sizeof(uint16_t);

		for(int i = 0; i != maxLoop; ++i){
			AbstractSmartcontractTransaction* trx = this->smartcontractTransactions->get(i);
			total += trx->binarySize();
		}
	}

	total += this->rewardBase->binarySize();

	return total;
}

void BlockBody::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->rewardBase);

	out->putLong(this->nonce);

	{
		int maxLoop = this->controlTransactions->size();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_CONTROL_TRX);
		out->putShort(maxLoop);

		for(int i = 0; i != maxLoop; ++i){
			AbstractControlTransaction* trx = this->controlTransactions->get(i);
			trx->toBinary(out);
		}
	}

	{
		int maxLoop = this->iccTransactions->size();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_ICC_TRX);
		out->putShort(maxLoop);

		for(int i = 0; i != maxLoop; ++i){
			AbstractInterChainCommunicationTansaction* trx = this->iccTransactions->get(i);
			trx->toBinary(out);
		}
	}

	{
		int maxLoop = this->balanceTransactions->size();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_BALANCE_TRX);
		out->putShort(maxLoop);

		for(int i = 0; i != maxLoop; ++i){
			AbstractBalanceTransaction* trx = this->balanceTransactions->get(i);
			trx->toBinary(out);
		}
	}

	{
		int maxLoop = this->smartcontractTransactions->size();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_SMARTCONTRACT_TRX);
		out->putShort(maxLoop);

		for(int i = 0; i != maxLoop; ++i){
			AbstractSmartcontractTransaction* trx = this->smartcontractTransactions->get(i);
			trx->toBinary(out);
		}
	}

	this->rewardBase->toBinary(out);
}

BlockBody* BlockBody::fromBinary(ByteBuffer *in) {
	BlockBody* body = new BlockBody(0L); __STP(body);

	body->nonce = in->getLong();

	{
		int maxLoop = in->getShort();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_CONTROL_TRX);

		for(int i = 0; i != maxLoop; ++i){
			AbstractBlockchainTransaction* trx = AbstractBalanceTransaction::createFromBinary(in); __STP(trx);
			AbstractControlTransaction* ctrx = dynamic_cast<AbstractControlTransaction*>(trx);
			BinaryUtils::checkNotNull(ctrx);

			body->addControlTransaction(ctrx);
		}
	}

	{
		int maxLoop = in->getShort();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_ICC_TRX);

		for(int i = 0; i != maxLoop; ++i){
			AbstractBlockchainTransaction* trx = AbstractBalanceTransaction::createFromBinary(in); __STP(trx);
			AbstractInterChainCommunicationTansaction* icctrx = dynamic_cast<AbstractInterChainCommunicationTansaction*>(trx);
			BinaryUtils::checkNotNull(icctrx);

			body->addInterChainCommunicationTransaction(icctrx);
		}
	}

	{
		int maxLoop = in->getShort();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_BALANCE_TRX);

		for(int i = 0; i != maxLoop; ++i){
			AbstractBlockchainTransaction* trx = AbstractBalanceTransaction::createFromBinary(in); __STP(trx);
			AbstractBalanceTransaction* btrx = dynamic_cast<AbstractBalanceTransaction*>(trx);
			BinaryUtils::checkNotNull(btrx);

			body->addBalanceTransaction(btrx);
		}
	}

	{
		int maxLoop = in->getShort();
		BinaryUtils::checkUShortRange(maxLoop, 0, MAX_SMARTCONTRACT_TRX);

		for(int i = 0; i != maxLoop; ++i){
			AbstractBlockchainTransaction* trx = AbstractBalanceTransaction::createFromBinary(in); __STP(trx);
			AbstractSmartcontractTransaction* sctrx = dynamic_cast<AbstractSmartcontractTransaction*>(trx);

			body->addSmartcontractTransaction(sctrx);
		}
	}

	delete body->rewardBase, body->rewardBase = nullptr;
	body->rewardBase = BlockRewordBase::createFromBinary(in);
	BinaryUtils::checkNotNull(body->rewardBase);

	body->build();

	return __STP_MV(body);
}

IBlockObject* BlockBody::copyData() const noexcept {
	BlockBody* body = new BlockBody(0L);

	body->nonce = this->nonce;

	{
		int maxLoop = this->controlTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractControlTransaction* trx = this->controlTransactions->get(i);

			body->addControlTransaction(trx);
		}
	}
	{
		int maxLoop = this->iccTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractInterChainCommunicationTansaction* trx = this->iccTransactions->get(i);

			body->addInterChainCommunicationTransaction(trx);
		}
	}
	{
		int maxLoop = this->balanceTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractBalanceTransaction* trx = this->balanceTransactions->get(i);

			body->addBalanceTransaction(trx);
		}
	}
	{
		int maxLoop = this->smartcontractTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractSmartcontractTransaction* trx = this->smartcontractTransactions->get(i);

			body->addSmartcontractTransaction(trx);
		}
	}

	// copy reword base
	delete body->rewardBase;
	body->rewardBase = new BlockRewordBase(*this->rewardBase);

	body->build();

	return body;
}

BalanceUnit BlockBody::getTotalFee() const noexcept {
	BalanceUnit fee(0L);
	{
		int maxLoop = this->balanceTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractBalanceTransaction* trx = this->balanceTransactions->get(i);
			fee += trx->getFee();
		}
	}
	{
		int maxLoop = this->iccTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractInterChainCommunicationTansaction* trx = this->iccTransactions->get(i);
			fee += trx->getFee();
		}
	}
	{
		int maxLoop = this->controlTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractBlockchainTransaction* trx = this->controlTransactions->get(i);
			fee += trx->getFee();
		}
	}
	{
		int maxLoop = this->smartcontractTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractSmartcontractTransaction* trx = this->smartcontractTransactions->get(i);
			fee += trx->getFee();
		}
	}

	return fee;
}

const AbstractBlockchainTransaction* BlockBody::getControlTransaction(const TransactionId *trxId) const noexcept {
	const AbstractBlockchainTransaction* ret = nullptr;

	int maxLoop = this->controlTransactions->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockchainTransaction* trx = this->controlTransactions->get(i);

		if(trxId->equals(trx->getTransactionId())){
			ret = trx;
			break;
		}
	}

	return ret;
}

const AbstractBalanceTransaction* BlockBody::getBalanceTransaction(const TransactionId *trxId) const noexcept {
	const AbstractBalanceTransaction* ret = nullptr;

	int maxLoop = this->balanceTransactions->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBalanceTransaction* trx = this->balanceTransactions->get(i);

		if(trxId->equals(trx->getTransactionId())){
			ret = trx;
			break;
		}
	}

	return ret;
}

const AbstractInterChainCommunicationTansaction* BlockBody::getInterChainCommunicationTansaction(const TransactionId *trxId) const noexcept {
	const AbstractInterChainCommunicationTansaction* ret = nullptr;

	int maxLoop = this->iccTransactions->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractInterChainCommunicationTansaction* trx = this->iccTransactions->get(i);

		if(trxId->equals(trx->getTransactionId())){
			ret = trx;
			break;
		}
	}

	return ret;
}

const AbstractSmartcontractTransaction* BlockBody::getSmartcontractTransaction(const TransactionId *trxId) const noexcept {
	const AbstractSmartcontractTransaction* ret = nullptr;

	int maxLoop = this->smartcontractTransactions->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractSmartcontractTransaction* trx = this->smartcontractTransactions->get(i);

		if(trxId->equals(trx->getTransactionId())){
			ret = trx;
			break;
		}
	}

	return ret;
}

void BlockBody::resetMerkle() noexcept {
	delete this->merkleRoot, this->merkleRoot = nullptr;
	delete this->tree, this->tree = nullptr;
}

UnicodeString* BlockBody::toStatString() const noexcept {
	UnicodeString* str = new UnicodeString(L"");

	//UnicodeString* rootstr = this->merkleRoot->toString(); __STP(rootstr);
	//str->append(rootstr);
	str->append(L" [ ");

	str->append(L" CTRL: ").append((int64_t)this->controlTransactions->size());
	str->append(L" ICC: ").append((int64_t)this->iccTransactions->size());
	str->append(L" BL: ").append((int64_t)this->balanceTransactions->size());
	str->append(L" SMRT: ").append((int64_t)this->smartcontractTransactions->size());
	str->append(L" ]");

	return str;
}

ArrayList<const VoteBlockTransaction>* BlockBody::findVoteBlockTransaction(const UtxoId *utxoId) const noexcept {
	ArrayList<const VoteBlockTransaction>* ret = new ArrayList<const VoteBlockTransaction>(); __STP(ret);

	int maxLoop = this->controlTransactions->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockchainTransaction* trx = this->controlTransactions->get(i);

		uint8_t type = trx->getType();
		if(type == AbstractBlockchainTransaction::TRX_TYPE_VOTE_BLOCK){
			VoteBlockTransaction* voteTrx = dynamic_cast<VoteBlockTransaction*>(trx);
			const TicketUtxoReference* ref = voteTrx->getTicketUtxoReference();
			const UtxoId* refUtxoId = ref->getUtxoId();

			if(refUtxoId->equals(utxoId)){
				ret->addElement(voteTrx);
			}
		}
	}

	return __STP_MV(ret);
}

ArrayList<const RevokeMissVotedTicket>* BlockBody::findRevokeMissVotedTicket(const UtxoId *utxoId) const noexcept {
	ArrayList<const RevokeMissVotedTicket>* ret = new ArrayList<const RevokeMissVotedTicket>(); __STP(ret);

	int maxLoop = this->controlTransactions->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockchainTransaction* trx = this->controlTransactions->get(i);

		uint8_t type = trx->getType();
		if(type == AbstractBlockchainTransaction::TRX_TYPE_REVOKE_MISS_VOTED_TICKET){
			RevokeMissVotedTicket* revokeTrx = dynamic_cast<RevokeMissVotedTicket*>(trx);
			const TicketVotedUtxoReference* ref = revokeTrx->getTicketVotedUtxoReference();
			const UtxoId* refUtxoId = ref->getUtxoId();

			if(refUtxoId->equals(utxoId)){
				ret->addElement(revokeTrx);
			}
		}
	}

	return __STP_MV(ret);
}

ArrayList<const RevokeMissedTicket>* BlockBody::findRevokeMissedTicket(const UtxoId *utxoId) const noexcept {
	ArrayList<const RevokeMissedTicket>* ret = new ArrayList<const RevokeMissedTicket>(); __STP(ret);

	int maxLoop = this->controlTransactions->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockchainTransaction* trx = this->controlTransactions->get(i);

		uint8_t type = trx->getType();
		if(type == AbstractBlockchainTransaction::TRX_TYPE_REVOKE_MISSED_TICKET){
			RevokeMissedTicket* revokeTrx = dynamic_cast<RevokeMissedTicket*>(trx);
			const TicketUtxoReference* ref = revokeTrx->getTicketUtxoReference();
			const UtxoId* refUtxoId = ref->getUtxoId();

			if(refUtxoId->equals(utxoId)){
				ret->addElement(revokeTrx);
			}
		}
	}

	return __STP_MV(ret);
}

MerkleCertificate* BlockBody::makeCertificate(const Abstract32BytesId *b) const noexcept {
	return this->tree->makeCertificate(b->toArray(), b->size());
}

MerkleCertificate* BlockBody::makeCertificate(const char *hash, int size) const noexcept {
	return this->tree->makeCertificate(hash, size);

}

OmittedBlockBody* BlockBody::toOmittedBlockBody() const {
	OmittedBlockBody* obody = new OmittedBlockBody(); __STP(obody);

	obody->setNonce(this->nonce);

	{
		int maxLoop = this->controlTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractControlTransaction* trx = this->controlTransactions->get(i);
			const TransactionId* trxId = trx->getTransactionId();

			obody->addControlTransaction(trxId);
		}
	}

	{
		int maxLoop = this->iccTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractInterChainCommunicationTansaction* trx = this->iccTransactions->get(i);
			const TransactionId* trxId = trx->getTransactionId();

			obody->addInterChainCommunicationTransaction(trxId);
		}
	}

	{
		int maxLoop = this->balanceTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractBalanceTransaction* trx = this->balanceTransactions->get(i);
			const TransactionId* trxId = trx->getTransactionId();

			obody->addBalanceTransaction(trxId);
		}
	}

	{
		int maxLoop = this->smartcontractTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractSmartcontractTransaction* trx = this->smartcontractTransactions->get(i);
			const TransactionId* trxId = trx->getTransactionId();

			obody->addSmartContractTransaction(trxId);
		}
	}

	obody->setRewordBase(this->rewardBase);

	return __STP_MV(obody);
}

void BlockBody::setBlockRewordBase(const BlockRewordBase *rewardBase) {
	delete this->rewardBase;
	this->rewardBase = new BlockRewordBase(*rewardBase);
}

void BlockBody::visitTransactions(ITransactionVisitor *visitor, const Block* block) const {
	{
		int maxLoop = this->controlTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractControlTransaction* trx = this->controlTransactions->get(i);

			visitor->visit(trx, block);
		}
	}
	{
		int maxLoop = this->iccTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractInterChainCommunicationTansaction* trx = this->iccTransactions->get(i);

			visitor->visit(trx, block);
		}
	}
	{
		int maxLoop = this->balanceTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractBalanceTransaction* trx = this->balanceTransactions->get(i);

			visitor->visit(trx, block);
		}
	}
	{
		int maxLoop = this->smartcontractTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractSmartcontractTransaction* trx = this->smartcontractTransactions->get(i);

			visitor->visit(trx, block);
		}
	}

	this->rewardBase->visitTransactions(visitor, block);
}


#ifdef __DEBUG__

void BlockBody::assertTransactionsBinary() {
	{
		int maxLoop = this->controlTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractControlTransaction* trx = this->controlTransactions->get(i);

			bool bl = checkTransaction(trx);
			assert(bl == true);
		}
	}
	{
		int maxLoop = this->iccTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractInterChainCommunicationTansaction* trx = this->iccTransactions->get(i);

			bool bl = checkTransaction(trx);
			assert(bl == true);
		}
	}
	{
		int maxLoop = this->balanceTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractBalanceTransaction* trx = this->balanceTransactions->get(i);

			bool bl = checkTransaction(trx);
			assert(bl == true);
		}
	}
	{
		int maxLoop = this->smartcontractTransactions->size();
		for(int i = 0; i != maxLoop; ++i){
			AbstractSmartcontractTransaction* trx = this->smartcontractTransactions->get(i);
			const TransactionId* trxId = trx->getTransactionId();

			bool bl = checkTransaction(trx);
			assert(bl == true);
		}
	}

}

bool BlockBody::checkTransaction(const AbstractBlockchainTransaction *trx) {
	const TransactionId* trxId = trx->getTransactionId();

	int size = trx->binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(size, true); __STP(buff);
	trx->toBinary(buff);

	buff->position(0);

	AbstractBlockchainTransaction* bintrx = AbstractBlockchainTransaction::createFromBinary(buff); __STP(bintrx);

	const TransactionId* bintrxId = bintrx->getTransactionId();

	bool bl = trxId->equals(bintrxId);
	return bl;
}

#endif

} /* namespace codablecash */
