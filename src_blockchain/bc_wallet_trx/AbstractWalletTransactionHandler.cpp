/*
 * AbstractWalletTransactionHandler.cpp
 *
 *  Created on: 2023/02/25
 *      Author: iizuka
 */

#include "bc_wallet_trx/AbstractWalletTransactionHandler.h"

#include "bc_wallet_trx_repo/WalletAccountTrxRepository.h"

#include "bc_wallet/WalletAccount.h"

#include "bc_trx/AbstractBlockchainTransaction.h"
#include "bc_trx/UtxoId.h"
#include "bc_trx/AbstractUtxoReference.h"
#include "bc_trx/AbstractUtxo.h"

#include "bc_base/AddressDescriptor.h"

#include "base/StackRelease.h"


namespace codablecash {

AbstractWalletTransactionHandler::AbstractWalletTransactionHandler(WalletAccount* account) {
	this->account = account;
}

AbstractWalletTransactionHandler::~AbstractWalletTransactionHandler() {
	this->account = nullptr;
}

void AbstractWalletTransactionHandler::__importTransaction(const AbstractBlockchainTransaction *__trx) {
	WalletAccountTrxRepository* trxRepo = this->account->getWalletAccountTrxRepository();

	bool imported = false;
	// remove used utxos
	{
		int maxLoop = __trx->getUtxoReferenceSize();
		for(int i = 0; i != maxLoop; ++i){
			AbstractUtxoReference* utxoRef = __trx->getUtxoReference(i);
			const UtxoId* utxoId = utxoRef->getUtxoId();

			if(utxoId == nullptr){
				continue;
			}

			AbstractUtxo* finded = trxRepo->getUtxo(utxoId); __STP(finded);
			if(finded != nullptr){
				trxRepo->removeUtxo(utxoId);
				imported = true;
			}
		}
	}

	// add utxo
	int maxLoop = __trx->getUtxoSize();
	for(int i = 0; i != maxLoop; ++i){
		AbstractUtxo* utxo = __trx->getUtxo(i);

		const AddressDescriptor* addressDesc = utxo->getAddress();

		if(this->account->hasAddress(addressDesc)){
			trxRepo->importUtxo(utxo);
			imported = true;
		}
	}

	if(imported){
		trxRepo->importTransaction(__trx);
	}
}

} /* namespace codablecash */
