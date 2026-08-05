/*
 * BalanceTransactionWalletHandler.h
 *
 *  Created on: 2023/02/25
 *      Author: iizuka
 */

#ifndef BC_WALLET_TRX_BALANCETRANSACTIONWALLETHANDLER_H_
#define BC_WALLET_TRX_BALANCETRANSACTIONWALLETHANDLER_H_

#include "bc_wallet_trx/AbstractWalletTransactionHandler.h"

#include "base/ArrayList.h"

using namespace alinous;

namespace codablecash {

class BalanceTransferTransaction;
class AddressDescriptor;
class BalanceUnit;
class IWalletDataEncoder;
class ITransactionBuilderContext;
class IUtxoCollector;
class ArrayUtxoFinder;
class HdWalleMuSigSignerProvidor;
class DestAddressPair;

class BalanceTransactionWalletHandler: public AbstractWalletTransactionHandler {
public:
	explicit BalanceTransactionWalletHandler(WalletAccount* account);
	virtual ~BalanceTransactionWalletHandler();

	BalanceTransferTransaction* createTransaction(ArrayList<DestAddressPair>* dest
			, const BalanceUnit& feeRate, bool feeIncluded, const IWalletDataEncoder* encoder, ITransactionBuilderContext* context);
	BalanceTransferTransaction* createTransaction(const AddressDescriptor* dest, const BalanceUnit& amount
			, const BalanceUnit& feeRate, bool feeIncluded, const IWalletDataEncoder* encoder, ITransactionBuilderContext* context);


	virtual void importTransaction(const AbstractBlockchainTransaction *trx);

private:
	BalanceTransferTransaction* createFeeIncludedTransaction(ArrayList<DestAddressPair>* dest
			, const BalanceUnit& feeRate, const IWalletDataEncoder* encoder, ITransactionBuilderContext* context);

	void collectUtxoRefs(BalanceTransferTransaction* trx, BalanceUnit& amount, const BalanceUnit& feeRate, IUtxoCollector* collector
			, ArrayUtxoFinder* utxoFinder, HdWalleMuSigSignerProvidor* musigProvidor, const IWalletDataEncoder* encoder);
};


class DestAddressPair {
public:
	DestAddressPair(const AddressDescriptor* dest, const BalanceUnit* amount);
	~DestAddressPair();

	const AddressDescriptor* getDest() const noexcept {
		return this->dest;
	}
	const BalanceUnit* getAmount() const noexcept {
		return this->amount;
	}
private:
	AddressDescriptor* dest;
	BalanceUnit* amount;
};

} /* namespace codablecash */

#endif /* BC_WALLET_TRX_BALANCETRANSACTIONWALLETHANDLER_H_ */
