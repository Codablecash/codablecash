/*
 * TransactionTransferData.cpp
 *
 *  Created on: 2023/09/24
 *      Author: iizuka
 */

#include "data_history_data/TransactionTransferData.h"

#include "bc_trx/AbstractBlockchainTransaction.h"

#include "base_io/ByteBuffer.h"

#include "bc_base/BinaryUtils.h"

#include "data_history/TransferedDataId.h"

#include "bc_trx/TransactionId.h"
namespace codablecash {

TransactionTransferData::TransactionTransferData(const TransactionTransferData &inst) : AbstractTransferedData(inst) {
	this->trx = inst.trx != nullptr ? dynamic_cast<AbstractBlockchainTransaction*>(inst.trx->copyData()) : nullptr;
}

TransactionTransferData::TransactionTransferData() : AbstractTransferedData(DATA_TRANSACTION) {
	this->trx = nullptr;
}

TransactionTransferData::~TransactionTransferData() {
	delete this->trx;
}

int TransactionTransferData::binarySize() const {
	BinaryUtils::checkNotNull(this->trx);

	int total = sizeof(uint8_t);
	total += this->trx->binarySize();

	return total;
}

void TransactionTransferData::toBinary(ByteBuffer *out) const {
	BinaryUtils::checkNotNull(this->trx);

	out->put(this->type);
	this->trx->toBinary(out);
}

void TransactionTransferData::fromBinary(ByteBuffer *in) {
	this->trx = AbstractBlockchainTransaction::createFromBinary(in);

	BinaryUtils::checkNotNull(this->trx);
}

IBlockObject* TransactionTransferData::copyData() const noexcept {
	return new TransactionTransferData(*this);
}

void TransactionTransferData::setTransaction(const AbstractBlockchainTransaction *trx) noexcept {
	delete this->trx;
	this->trx = dynamic_cast<AbstractBlockchainTransaction*>(trx->copyData());
}

TransferedDataId* TransactionTransferData::getTransferedDataId() const {
	const TransactionId* id = this->trx->getTransactionId();

	TransferedDataId* dataId = new TransferedDataId(id->toArray(), id->size());
	return dataId;
}

} /* namespace codablecash */
