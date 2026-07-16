/*
 * NopInterChainCommunicationTransaction.cpp
 *
 *  Created on: 2023/12/04
 *      Author: iizuka
 */

#include "bc_trx/NopInterChainCommunicationTransaction.h"
#include "bc_trx/TransactionVersion.h"
#include "bc_trx/TransactionId.h"
#include "bc_trx/AbstractUtxo.h"

#include "bc_base/BalanceUnit.h"
#include "bc_base/BinaryUtils.h"

#include "base/StackRelease.h"

#include "crypto/Sha256.h"



#include "base_timestamp/SystemTimestamp.h"


namespace codablecash {

uint64_t NopInterChainCommunicationTransaction::serial = 1;

NopInterChainCommunicationTransaction::NopInterChainCommunicationTransaction(const NopInterChainCommunicationTransaction &inst)
		: AbstractInterChainCommunicationTansaction(inst) {
	this->nonce = inst.nonce;
	this->utxoList = new ArrayList<AbstractUtxo>();

	int maxLoop = inst.utxoList->size();
	for(int i = 0; i != maxLoop; ++i){
		const AbstractUtxo* utxo = inst.utxoList->get(i);

		this->utxoList->addElement(dynamic_cast<AbstractUtxo*>(utxo->copyData()));
	}
}

NopInterChainCommunicationTransaction::NopInterChainCommunicationTransaction() : AbstractInterChainCommunicationTansaction() {
	this->nonce = serial++;
	this->utxoList = new ArrayList<AbstractUtxo>();
}

NopInterChainCommunicationTransaction::~NopInterChainCommunicationTransaction() {
	this->utxoList->deleteElements();
	delete this->utxoList;
}

uint8_t NopInterChainCommunicationTransaction::getType() const noexcept {
	return TRX_TYPE_ICC_NOP;
}

int NopInterChainCommunicationTransaction::binarySize() const {
	BinaryUtils::checkNotNull(this->timestamp);

	int total = sizeof(uint8_t);
	total += this->version->binarySize();
	total += this->timestamp->binarySize();
	total += sizeof(this->nonce);

	int maxLoop = this->utxoList->size();
	total += sizeof(uint16_t);

	for(int i = 0; i != maxLoop; ++i){
		const AbstractUtxo* utxo = this->utxoList->get(i);

		total += utxo->binarySize();
	}

	return total;
}

void NopInterChainCommunicationTransaction::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->timestamp);

	out->put(getType());

	this->version->toBinary(out);
	this->timestamp->toBinary(out);
	out->putLong(this->nonce);

	int maxLoop = this->utxoList->size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		const AbstractUtxo* utxo = this->utxoList->get(i);

		utxo->toBinary(out);
	}
}

void NopInterChainCommunicationTransaction::fromBinary(ByteBuffer *in) {
	delete this->version;
	this->version = TransactionVersion::createFromBinary(in);

	delete this->timestamp;
	this->timestamp = SystemTimestamp::fromBinary(in);
	this->nonce = in->getLong();

	int maxLoop = in->getShort();
	for(int i = 0; i != maxLoop; ++i){
		AbstractUtxo* utxo = AbstractUtxo::createFromBinary(in); __STP(utxo);
		BinaryUtils::checkNotNull(utxo);

		this->utxoList->addElement(__STP_MV(utxo));
	}
}

void NopInterChainCommunicationTransaction::build() {
	BinaryUtils::checkNotNull(this->timestamp);

	setUtxoNonce();

	int capacity = sizeof(uint8_t) + this->version->binarySize() + this->timestamp->binarySize() + sizeof(this->nonce);

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);
	buff->put(getType());

	this->version->toBinary(buff);
	this->timestamp->toBinary(buff);
	buff->putLong(this->nonce);

	buff->position(0);

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);

	delete this->trxId;
	this->trxId = new TransactionId((const char*)sha->array(), sha->limit());
}

void NopInterChainCommunicationTransaction::setUtxoNonce() noexcept {
	BinaryUtils::checkNotNull(this->timestamp);
	BinaryUtils::checkNotNull(this->version);

	int capacity = sizeof(uint8_t) + this->version->binarySize() + this->timestamp->binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);
	buff->put(getType());
	this->version->toBinary(buff);
	this->timestamp->toBinary(buff);


	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);
	const uint8_t* data = sha->array();

	int maxLoop = this->utxoList->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractUtxo* utxo = this->utxoList->get(i);

		utxo->setNonce(data, i);

		utxo->build();
	}
}

IBlockObject* NopInterChainCommunicationTransaction::copyData() const noexcept {
	return new NopInterChainCommunicationTransaction(*this);
}

BalanceUnit NopInterChainCommunicationTransaction::getFee() const noexcept {
	return BalanceUnit(0);
}

BalanceUnit NopInterChainCommunicationTransaction::getFeeRate() const noexcept {
	return BalanceUnit(0);
}

int NopInterChainCommunicationTransaction::getUtxoSize() const noexcept {
	return this->utxoList->size();
}

AbstractUtxo* NopInterChainCommunicationTransaction::getUtxo(int i) const noexcept {
	return this->utxoList->get(i);
}

int NopInterChainCommunicationTransaction::getUtxoReferenceSize() const noexcept {
	return 0;
}

AbstractUtxoReference* NopInterChainCommunicationTransaction::getUtxoReference(int i) const noexcept {
	return nullptr;
}

bool NopInterChainCommunicationTransaction::validateOnAccept(MemPoolTransaction *memTrx, IStatusCacheContext *context) const {
	return true;
}

TrxValidationResult NopInterChainCommunicationTransaction::validateFinal(
		const BlockHeader *header, MemPoolTransaction *memTrx,
		IStatusCacheContext *context) const {
	return TrxValidationResult::OK;
}

bool NopInterChainCommunicationTransaction::checkFilter(const ArrayList<BloomFilter1024> *filtersList) const {
	return true;
}

void NopInterChainCommunicationTransaction::addutxo(const AbstractUtxo *utxo) noexcept {
	this->utxoList->addElement(dynamic_cast<AbstractUtxo*>(utxo->copyData()));
}
} /* namespace codablecash */
