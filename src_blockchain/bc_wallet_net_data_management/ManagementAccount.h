/*
 * ManagementAccount.h
 *
 *  Created on: Jul 16, 2025
 *      Author: iizuka
 */

#ifndef BC_WALLET_NET_DATA_MANAGEMENT_MANAGEMENTACCOUNT_H_
#define BC_WALLET_NET_DATA_MANAGEMENT_MANAGEMENTACCOUNT_H_

#include "bc_trx/IUtxoRefChecker.h"

#include "base/ArrayList.h"

using namespace alinous;

namespace alinous {
class SysMutex;

}  // namespace alinous

namespace codablecash {

class ManagementTransactionsHistory;
class ManagedUtxoCache;
class AbstractBlockchainTransaction;
class ManagedUtxoCacheRecord;
class IAddressChecker;
class TransactionId;


class ManagementAccount : public IUtxoRefChecker {
public:
	explicit ManagementAccount(int storeType);
	virtual ~ManagementAccount();

	void reset() noexcept;

	virtual bool checkUtxo(const AbstractUtxoReference* ref) const;

	void addTransaction(const AbstractBlockchainTransaction* trx, const IAddressChecker *addressChecker);
	bool hasTransaction(const TransactionId *trxId) const;


	void importOtherAccount(const ManagementAccount* other);

	const ArrayList<ManagedUtxoCacheRecord>* getUtxoList() const noexcept;

	void importHdWalletAccount();

	SysMutex* getMutex() const noexcept {
		return this->mutex;
	}
private:
	ManagementTransactionsHistory* trxHistory;
	ManagedUtxoCache* utxoCache;

	int storeType; // FINALIZED, UNFINALIZED, or MEMPOOL

	SysMutex* mutex;
};

} /* namespace codablecash */

#endif /* BC_WALLET_NET_DATA_MANAGEMENT_MANAGEMENTACCOUNT_H_ */
