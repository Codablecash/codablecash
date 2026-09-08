/*
 * AbstractUtxo.cpp
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#include "bc_trx/AbstractUtxo.h"
#include "bc_trx/UtxoId.h"

#include "base_io/ByteBuffer.h"

#include "bc_trx_balance/BalanceUtxo.h"

#include "bc_finalizer_trx/TicketUtxo.h"
#include "bc_finalizer_trx/TicketVotedUtxo.h"

#include "bc_status_cache_context/RemovedDummyUtxo.h"

#include "crypto/Sha256.h"

#include "base/StackRelease.h"

#include "bc_trx_icc/RemoteBalanceUtxo.h"


namespace codablecash {

AbstractUtxo::AbstractUtxo(const AbstractUtxo &inst) {
	this->utxoId = inst.utxoId != nullptr ? dynamic_cast<UtxoId*>(inst.utxoId->copyData()) : nullptr;
	Mem::memset(this->nonce, 0, 32);
}

AbstractUtxo::AbstractUtxo() {
	this->utxoId = nullptr;
	Mem::memset(this->nonce, 0, 32);
}

AbstractUtxo::~AbstractUtxo() {
	delete this->utxoId;
}

AbstractUtxo* AbstractUtxo::createFromBinary(ByteBuffer *in) {
	AbstractUtxo* ret = nullptr;

	uint8_t type = in->get();
	switch(type){
	case AbstractUtxo::TRX_UTXO_BALANCE:
		ret = new BalanceUtxo();
		break;
	case AbstractUtxo::TRX_UTXO_TICKET:
		ret = new TicketUtxo();
		break;
	case AbstractUtxo::TRX_UTXO_VOTED_TICKET:
		ret = new TicketVotedUtxo();
		break;
	case AbstractUtxo::TRX_UTXO_REMOTE_BALANCE:
		ret = new RemoteBalanceUtxo();
		break;
	case AbstractUtxo::TRX_UTXO_REMOVED_DUMMY:
		ret = new RemovedDummyUtxo();
		break;
	default:
		return nullptr;
	}

	ret->fromBinary(in);

	return ret;
}

bool AbstractUtxo::equals(const AbstractUtxo *other) const noexcept {
	return this->utxoId->equals(other->utxoId);
}

void AbstractUtxo::setNonce(const uint8_t *data32bytes, int index) {
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(32 + sizeof(int32_t), true); __STP(buff);
	buff->put(data32bytes, 32);
	buff->putInt(index);
	buff->position(0);

	ByteBuffer* sha = Sha256::sha256(buff, true); __STP(sha);
	sha->position(0);

	Mem::memcpy(this->nonce, sha->array(), 32);

}

} /* namespace codablecash */
