/*
 * BalanceTransferTransaction.cpp
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#include "bc_trx_balance/BalanceTransferTransaction.h"
#include "bc_trx_balance/InputUtxoCollection.h"
#include "bc_trx_balance/BalanceUtxo.h"
#include "bc_trx_balance/BalanceUtxoSign.h"
#include "bc_trx_balance/BalanceUtxoReference.h"

#include "bc_trx/TransactionId.h"
#include "bc_trx/TransactionVersion.h"

#include "bc_base/BinaryUtils.h"
#include "bc_base/AddressDescriptor.h"

#include "base/StackRelease.h"

#include "base_timestamp/SystemTimestamp.h"

#include "crypto/Sha256.h"

#include "musig/MuSig.h"

#include "ecda/Secp256k1CompressedPoint.h"
#include "ecda/Secp256k1Point.h"


namespace codablecash {

BalanceTransferTransaction::BalanceTransferTransaction(const BalanceTransferTransaction& inst) :
		AbstractBalanceTransaction(inst) {
	this->inputs = new InputUtxoCollection(*inst.inputs);

	int maxLoop = inst.list.size();
	for(int i = 0; i != maxLoop; ++i){
		BalanceUtxo* utxo = inst.list.get(i);
		addBalanceUtxo(utxo);
	}

	this->fee = inst.fee;

	this->signature = inst.signature != nullptr ? dynamic_cast<BalanceUtxoSign*>(inst.signature->copyData()) : nullptr;
}

BalanceTransferTransaction::BalanceTransferTransaction() {
	this->inputs = new InputUtxoCollection();
	this->signature = nullptr;
}

BalanceTransferTransaction::~BalanceTransferTransaction() {
	delete this->inputs;
	this->list.deleteElements();
	delete this->signature;
}

int BalanceTransferTransaction::binarySize() const {
//	BinaryUtils::checkNotNull(this->signature);

	int total = __binarySize();

	total += sizeof(uint8_t);
	if(this->signature != nullptr){
		total += this->signature->binarySize();
	}

	return total;
}

int BalanceTransferTransaction::__binarySize() const {
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->timestamp);

	int total = sizeof(uint8_t);
	total += this->version->binarySize();
	total += this->timestamp->binarySize();

	total += this->inputs->binarySize();

	total += sizeof(uint16_t);
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		BalanceUtxo* utxo = this->list.get(i);
		total += utxo->binarySize();
	}

	total += this->fee.binarySize();

	return total;
}

void BalanceTransferTransaction::toBinary(ByteBuffer *out) const {
	// BinaryUtils::checkNotNull(this->signature);

	__toBinary(out);

	bool bl = this->signature != nullptr;
	out->put(bl ? 1 : 0);

	if(bl){
		this->signature->toBinary(out);
	}
}

void BalanceTransferTransaction::__toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->version);
	BinaryUtils::checkNotNull(this->timestamp);

	out->put(getType());
	this->version->toBinary(out);
	this->timestamp->toBinary(out);

	this->inputs->toBinary(out);

	int maxLoop = this->list.size();
	out->putShort(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		BalanceUtxo* utxo = this->list.get(i);
		utxo->toBinary(out);
	}

	this->fee.toBinary(out);
}

void BalanceTransferTransaction::fromBinary(ByteBuffer *in) {
	delete this->version, this->version = nullptr;
	this->version = TransactionVersion::createFromBinary(in);

	delete this->timestamp, this->timestamp = nullptr;
	this->timestamp = SystemTimestamp::fromBinary(in);

	delete this->inputs;
	this->inputs = InputUtxoCollection::fromBinary(in);

	int maxLoop = in->getShort();
	BinaryUtils::checkUShortRange(maxLoop, 0, MAX_INPUT_ELEMENT);
	for(int i = 0; i != maxLoop; ++i){
		AbstractUtxo* u = AbstractUtxo::createFromBinary(in); __STP(u);

		BinaryUtils::checkUint8Value(u->getType(), AbstractUtxo::TRX_UTXO_BALANCE);
		BalanceUtxo* utxo = dynamic_cast<BalanceUtxo*>(u);
		addBalanceUtxo(utxo);
	}

	{
		BalanceUnit* unit = BalanceUnit::fromBinary(in); __STP(unit);
		this->fee = *unit;
	}

	uint8_t bl = in->get();
	if(bl > 0){
		this->signature = BalanceUtxoSign::fromBinary(in);
	}
}

void BalanceTransferTransaction::addBalanceUtxo(const BalanceUtxo *utxo) noexcept {
	BalanceUtxo* obj = dynamic_cast<BalanceUtxo*>(utxo->copyData());
	this->list.addElement(obj);
}

void BalanceTransferTransaction::addInputUtxoRef(const BalanceUtxoReference *ref) noexcept {
	this->inputs->addReference(ref);
}

void BalanceTransferTransaction::setFeeAmount(const BalanceUnit *fee, const AddressDescriptor *skipfeedesc) {
	setFeeAmount(fee);

	bool discounted = false;

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		BalanceUtxo* utxo = this->list.get(i);
		const AddressDescriptor* desc = utxo->getAddress();

		if(skipfeedesc != nullptr && desc->compareTo(skipfeedesc) == 0){
			continue;
		}

		if(!discounted){
			utxo->discountFee(*fee);
			discounted = true;
		}
	}
}

void BalanceTransferTransaction::setFeeAmount(const BalanceUnit* fee) noexcept {
	this->fee = *fee;
}

BalanceUnit BalanceTransferTransaction::getFeeRate() const noexcept {
	uint64_t size = binarySize();
	uint64_t lfee = this->fee.getAmount();

	uint64_t rate = lfee /size;

	return BalanceUnit(rate);
}

IBlockObject* BalanceTransferTransaction::copyData() const noexcept {
	BalanceTransferTransaction* trx = new BalanceTransferTransaction(*this);
	trx->build();
	return trx;
}

void BalanceTransferTransaction::build() {
	setUtxoNonce();

	int capacity = __binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);

	__toBinary(buff);
	buff->position(0);

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);

	delete this->trxId;
	this->trxId = new TransactionId((const char*)sha->array(), sha->limit());
}

void BalanceTransferTransaction::setUtxoNonce() noexcept {
	int capacity = sizeof(uint8_t) +  this->version->binarySize() + this->timestamp->binarySize() + this->inputs->binarySize();

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);
	buff->put(getType());
	this->version->toBinary(buff);
	this->timestamp->toBinary(buff);
	this->inputs->toBinary(buff);

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);
	const uint8_t* data = sha->array();

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		BalanceUtxo* utxo = this->list.get(i);

		utxo->setNonce(data, i);

		utxo->build();
	}
}

void BalanceTransferTransaction::sign(IMuSigSignerProvidor *providor, IUtxoFinder* finder) {
	assert(this->trxId != nullptr);

	int length = this->trxId->size();
	const char* data = this->trxId->toArray();

	MuSig sig = this->inputs->sign(providor, finder, data, length);
	const Secp256k1Point* R = sig.getR();
	Secp256k1CompressedPoint __R = Secp256k1CompressedPoint::compress(R);

	const BigInteger* s = sig.gets();

	delete this->signature;
	this->signature = new BalanceUtxoSign(&__R, s);
}

bool BalanceTransferTransaction::verify() const {
	assert(this->signature != nullptr);
	assert(this->trxId != nullptr);

	const Secp256k1CompressedPoint* R = this->signature->getR();
	Secp256k1Point ptR = R->decompress();
	const BigInteger* s = this->signature->gets();

	MuSig sig(ptR, *s);

	const ArrayList<BalanceUtxoReference>* list = this->inputs->getList();
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		BalanceUtxoReference* ref = list->get(i);

		const Secp256k1CompressedPoint* XiCompressed = ref->getXi();
		Secp256k1Point Xi = XiCompressed->decompress();

		sig.addXi(Xi);
	}

	int length = this->trxId->size();
	const char* data = this->trxId->toArray();

	bool bl = sig.verify(data, length);
	return bl;
}

const ArrayList<BalanceUtxoReference>* BalanceTransferTransaction::getUtxoRefList() const noexcept {
	return this->inputs->getList();
}

int BalanceTransferTransaction::getUtxoReferenceSize() const noexcept {
	return this->inputs->getList()->size();
}

AbstractUtxoReference* BalanceTransferTransaction::getUtxoReference(int i) const noexcept {
	return this->inputs->getList()->get(i);
}

bool BalanceTransferTransaction::validateOnAccept(MemPoolTransaction *memTrx, IStatusCacheContext *context) const {
	UtxoValidationResult utxoRes = validateUtxos(memTrx, context, this->fee);
	if(utxoRes == UtxoValidationResult::INVALID){
		return false;
	}

	return verify();
}

TrxValidationResult BalanceTransferTransaction::validateFinal(
		const BlockHeader *header, MemPoolTransaction *memTrx, IStatusCacheContext *context) const {
	UtxoValidationResult utxoRes = validateUtxos(memTrx, context, this->fee);
	if(utxoRes == UtxoValidationResult::INVALID){
		return TrxValidationResult::INVALID;
	}
	else if(utxoRes == UtxoValidationResult::ON_MEMORY){
		return TrxValidationResult::PENDING;
	}

	return TrxValidationResult::OK;
}

BalanceUnit BalanceTransferTransaction::getFee() const noexcept {
	return this->fee;
}

BalanceUnit BalanceTransferTransaction::getTotalOutputBalance() const {
	BalanceUnit total(0L);

	int maxLoop = getUtxoSize();
	for(int i = 0; i != maxLoop; ++i){
		AbstractUtxo* utxo = getUtxo(i);
		BalanceUnit amount = utxo->getAmount();

		total += amount;
	}

	total += this->fee;

	return total;
}

} /* namespace codablecash */
