/*
 * NetWalletAccountUtxoCollector.cpp
 *
 *  Created on: Jul 30, 2025
 *      Author: iizuka
 */

#include "bc_wallet_trx_base/NetWalletAccountUtxoCollector.h"
#include "bc_wallet_trx_base/NetWalletAccountUtxoArray.h"

#include "bc_wallet_net_data_management/ManagementAccount.h"
#include "bc_wallet_net_data_management/ManagedUtxoCacheRecord.h"

#include "bc_trx/AbstractUtxo.h"
#include "bc_trx_balance/BalanceUtxo.h"

#include "bc_wallet_net_data_management/ManagementAccountsCollection.h"

#include "base_thread/SysMutex.h"

#include "base_thread/StackUnlocker.h"
namespace codablecash {


/**
 * managementAccount is mempool version, which includes FINALIZED, UNFINALIZED, and MAMPOOL
 * @param managementAccount
 */
NetWalletAccountUtxoCollector::NetWalletAccountUtxoCollector(const ManagementAccount* managementAccount) {
	this->managementAccount = managementAccount;
	this->list = new ArrayList<NetWalletAccountUtxoArray>();

	int maxLoop = 3;
	for(int i = 0; i != maxLoop; ++i){
		NetWalletAccountUtxoArray* ar = new NetWalletAccountUtxoArray();
		this->list->addElement(ar);
	}

	this->nextUtxo = nullptr;

	this->utxoArrayIndex = 0;
}

NetWalletAccountUtxoCollector::~NetWalletAccountUtxoCollector() {
	this->managementAccount = nullptr;

	this->list->deleteElements();
	delete this->list;

	delete this->nextUtxo;
}

void NetWalletAccountUtxoCollector::init() {
	SysMutex* mutex = this->managementAccount->getMutex();
	StackUnlocker __lock(mutex, __FILE__, __LINE__);

	const ArrayList<ManagedUtxoCacheRecord>* utxolist = this->managementAccount->getUtxoList();

	int maxLoop = utxolist->size();
	for(int i = 0; i != maxLoop; ++i){
		ManagedUtxoCacheRecord* record = utxolist->get(i);

		const AbstractUtxo* utxo = record->getUtxo();
		int index = ManagementAccountsCollection::RECORD_SOTRE_TYPE_TO_INXED(record->getType()); // not record type

		if(utxo->getType() == AbstractUtxo::TRX_UTXO_BALANCE){
			NetWalletAccountUtxoArray* ar = this->list->get(index);
			ar->addBalanceUtxo(dynamic_cast<const BalanceUtxo*>(utxo));
		}
	}
}

bool NetWalletAccountUtxoCollector::hasNext() {
	bool ret = false;
	delete this->nextUtxo, this->nextUtxo = nullptr;

	while(this->utxoArrayIndex < 3){
		NetWalletAccountUtxoArray* ar = this->list->get(this->utxoArrayIndex);
		if(ar->hasNext()){
			const BalanceUtxo* utxo = ar->next();
			this->nextUtxo = new BalanceUtxo(*utxo);
			ret = true;
			break;
		}

		this->utxoArrayIndex++;
	}

	return ret;
}

BalanceUtxo* NetWalletAccountUtxoCollector::next() {
	return dynamic_cast<BalanceUtxo*>(this->nextUtxo->copyData());
}

} /* namespace codablecash */
