/*
 * RemoteUtxo.cpp
 *
 *  Created on: Aug 7, 2026
 *      Author: iizuka
 */

#include "bc_trx_icc/RemoteBalanceUtxo.h"

namespace codablecash {

RemoteBalanceUtxo::RemoteBalanceUtxo(const RemoteBalanceUtxo &inst) : BalanceUtxo(inst) {
}

RemoteBalanceUtxo::RemoteBalanceUtxo() : BalanceUtxo() {

}

RemoteBalanceUtxo::~RemoteBalanceUtxo() {

}

uint8_t RemoteBalanceUtxo::getType() const noexcept {
	return AbstractUtxo::TRX_UTXO_REMOTE_BALANCE;
}

bool RemoteBalanceUtxo::isRemote() const noexcept {
	return true;
}

IBlockObject* RemoteBalanceUtxo::copyData() const noexcept {
	return new RemoteBalanceUtxo(*this);
}

} /* namespace codablecash */
