/*
 * ManagementAccount.cpp
 *
 *  Created on: Jul 16, 2025
 *      Author: iizuka
 */

#include "bc_wallet_net_data_management/ManagementAccount.h"
#include "bc_wallet_net_data_management/ManagementTransactionsHistory.h"
#include "bc_wallet_net_data_management/ManagedUtxoCache.h"

#include "bc_trx/AbstractUtxoReference.h"
#include "bc_trx/AbstractBlockchainTransaction.h"
#include "bc_trx/IAddressChecker.h"
#include "bc_trx/AbstractUtxo.h"

#include "base_thread/SysMutex.h"

#include "base_thread/StackUnlocker.h"
namespace codablecash {

ManagementAccount::ManagementAccount(int storeType) {
	this->storeType = storeType;

	this->trxHistory = new ManagementTransactionsHistory();
	this->utxoCache = new ManagedUtxoCache();

	this->mutex = new SysMutex();

}

ManagementAccount::~ManagementAccount() {
	delete this->trxHistory;
	delete this->utxoCache;
	delete this->mutex;
}

void ManagementAccount::reset() noexcept {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	this->trxHistory->reset();
	this->utxoCache->reset();
}

bool ManagementAccount::checkUtxo(const AbstractUtxoReference *ref) const {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	const UtxoId* utxoId = ref->getUtxoId();

	return this->utxoCache->hasUtxo(utxoId);
}

void ManagementAccount::addTransaction(const AbstractBlockchainTransaction *trx, const IAddressChecker *addressChecker) {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	bool trxIsRelevant = false;
	const TransactionId* trxId = trx->getTransactionId();

	bool bl = this->trxHistory->hasTransaction(trxId);
	if(!bl){
		// utxo ref
		{
			int maxLoop = trx->getUtxoReferenceSize();
			for(int i = 0; i != maxLoop; ++i){
				const AbstractUtxoReference* ref = trx->getUtxoReference(i);
				const UtxoId* utxoId = ref->getUtxoId();

				bool hasUtxo = this->utxoCache->removeUtxo(utxoId);
				if(hasUtxo){
					trxIsRelevant = true;
				}
			}
		}

		// new utxo
		{
			int maxLoop = trx->getUtxoSize();
			for(int i = 0; i != maxLoop; ++i){
				const AbstractUtxo* utxo = trx->getUtxo(i);

				// check if the utxo is included in the wallet
				const AddressDescriptor* desc = utxo->getAddress();

				if(desc != nullptr && addressChecker->checkAddress(desc)){
					this->utxoCache->addUtxo(utxo, trxId, this->storeType);
					trxIsRelevant = true;
				}
			}
		}

		if(trxIsRelevant){
			this->trxHistory->addTransaction(trx);
		}
	}
}

void ManagementAccount::importOtherAccount(const ManagementAccount* other) {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	this->trxHistory->importOtherManagementTransactionsHistory(other->trxHistory);
	this->utxoCache->importOtherManagedUtxoCache(other->utxoCache);
}

const ArrayList<ManagedUtxoCacheRecord>* ManagementAccount::getUtxoList() const noexcept {
	return this->utxoCache->getUtxoList();
}

bool ManagementAccount::hasTransaction(const TransactionId *trxId) const {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	const AbstractBlockchainTransaction* trx = this->trxHistory->getTransaction(trxId);

	return trx != nullptr;
}

} /* namespace codablecash */
