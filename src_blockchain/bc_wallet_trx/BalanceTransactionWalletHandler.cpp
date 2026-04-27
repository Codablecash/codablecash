/*
 * BalanceTransactionWalletHandler.cpp
 *
 *  Created on: 2023/02/25
 *      Author: iizuka
 */

#include "bc_wallet_trx/BalanceTransactionWalletHandler.h"

#include "bc_wallet_trx_base/ITransactionBuilderContext.h"

#include "bc_wallet_trx_repo/WalletAccountTrxRepository.h"

#include "bc_base/BalanceUnit.h"
#include "bc_base/BalanceAddress.h"
#include "bc_base/AddressDescriptor.h"

#include "base/ArrayList.h"
#include "base/StackRelease.h"

#include "bc_trx_balance/BalanceTransferTransaction.h"
#include "bc_trx_balance/BalanceUtxo.h"
#include "bc_trx_balance/BalanceUtxoReference.h"
#include "bc_trx_balance/BalanceShortageException.h"

#include "bc_wallet/WalletAccount.h"
#include "bc_wallet/AbstractAddressStore.h"
#include "bc_wallet/ReceivingAddressStore.h"
#include "bc_wallet/ChangeAddressStore.h"
#include "bc_wallet/HdWalleMuSigSignerProvidor.h"

#include "musig/IMuSigSigner.h"

#include "ecda/Secp256k1Point.h"
#include "ecda/Secp256k1CompressedPoint.h"

#include "bc_wallet_trx_base/IUtxoCollector.h"

#include "bc_wallet_trx_base/ArrayUtxoFinder.h"

#include "bc/ExceptionThrower.h"
using namespace alinous;

namespace codablecash {

BalanceTransactionWalletHandler::BalanceTransactionWalletHandler(WalletAccount* account)
		:AbstractWalletTransactionHandler(account){

}

BalanceTransactionWalletHandler::~BalanceTransactionWalletHandler() {

}

BalanceTransferTransaction* BalanceTransactionWalletHandler::createTransaction(
		const AddressDescriptor *dest, const BalanceUnit amount,
		const BalanceUnit feeRate, bool feeIncluded,
		const IWalletDataEncoder *encoder,
		ITransactionBuilderContext *context) {
	if(feeIncluded){
		return createFeeIncludedTransaction(dest, amount, feeRate, encoder, context);
	}

	IUtxoCollector* collector = context->getUtxoCollector(); __STP(collector);
	ArrayUtxoFinder utxoFinder;

	HdWalleMuSigSignerProvidor* musigProvidor = context->getMusigSignProvidor(encoder); __STP(musigProvidor);

	BalanceTransferTransaction* trx = new BalanceTransferTransaction(); __STP(trx);
	// add output utxo
	{
		BalanceUtxo utxo(amount);
		utxo.setAddress(dest);
		trx->addBalanceUtxo(&utxo);
	}

	trx->build();
	trx->sign(musigProvidor, &utxoFinder);

	collectUtxoRefs(trx, amount, feeRate, collector, &utxoFinder, musigProvidor, encoder);

	trx->build();
	trx->sign(musigProvidor, &utxoFinder);

	return __STP_MV(trx);
}

void BalanceTransactionWalletHandler::collectUtxoRefs(BalanceTransferTransaction* trx, BalanceUnit amount, const BalanceUnit feeRate
		, IUtxoCollector *collector, ArrayUtxoFinder *utxoFinder, HdWalleMuSigSignerProvidor *musigProvidor, const IWalletDataEncoder* encoder) {
	int binSize = trx->binarySize();
	BalanceUnit fee = BalanceUnit(binSize) * feeRate;

	BalanceUnit required = fee + amount;
	BalanceUnit totalIn = utxoFinder->getTotalAmount();

	while(collector->hasNext() && totalIn.compareTo(&required) < 0){
		BalanceUtxo* utxo = collector->next(); __STP(utxo);
		const AddressDescriptor* desc = utxo->getAddress();

		BalanceUtxoReference ref;
		ref.setUtxoId(utxo->getId(), desc);

		IMuSigSigner* signer = musigProvidor->getSigner(desc); __STP(signer);
		Secp256k1Point pt = signer->getxG();
		Secp256k1CompressedPoint xi = Secp256k1CompressedPoint::compress(&pt);
		ref.setXi(&xi);

		trx->addInputUtxoRef(&ref);

		utxoFinder->addUtxo(utxo);

		// calc addfee
		int refbinSize = ref.binarySize();
		binSize += refbinSize;

		fee = BalanceUnit(binSize) * feeRate;
		required = fee + amount;
		totalIn = utxoFinder->getTotalAmount();

		trx->setFeeAmount(&fee);

		// add exchange address
		if(totalIn.compareTo(&required) > 0){
			if(trx->getUtxoSize() == 1){ // FIXME utxo size
				// add
				ChangeAddressStore* changeAddresses = this->account->getChangeAddresses();
				AddressDescriptor* changeDesc = changeAddresses->getNextChangeAddress(encoder); __STP(changeDesc);

				int utxobinSize = 0;
				{
					BalanceUnit tmp(0L);
					BalanceUtxo utxo(tmp);
					utxo.setAddress(changeDesc);
					trx->addBalanceUtxo(&utxo);

					utxo.build();
					utxobinSize = utxo.binarySize();
				}
				binSize += utxobinSize;

				// recalc
				fee = BalanceUnit(binSize) * feeRate;

				required = fee + amount;
				totalIn = utxoFinder->getTotalAmount();

				// setup
				trx->setFeeAmount(&fee);

				BalanceUnit diff = totalIn - required;
				{
					AbstractUtxo* u = trx->getUtxo(1);
					BalanceUtxo* utxo = dynamic_cast<BalanceUtxo*>(u);
					utxo->setAmount(diff);
				}
			}
			else{
				BalanceUnit diff = totalIn - required;
				AbstractUtxo* u = trx->getUtxo(1);
				BalanceUtxo* utxo = dynamic_cast<BalanceUtxo*>(u);

				utxo->setAmount(diff);
			}
		}
	}

	ExceptionThrower<BalanceShortageException>::throwExceptionIfCondition(totalIn.compareTo(&required) < 0, L"Wallet don't have enough balance.", __FILE__, __LINE__);
}



BalanceTransferTransaction* BalanceTransactionWalletHandler::createFeeIncludedTransaction(
		const AddressDescriptor *dest, const BalanceUnit amount,
		const BalanceUnit feeRate, const IWalletDataEncoder *encoder,
		ITransactionBuilderContext *context) {
	IUtxoCollector* collector = context->getUtxoCollector(); __STP(collector);
	ArrayUtxoFinder utxoFinder;

	HdWalleMuSigSignerProvidor* musigProvidor = context->getMusigSignProvidor(encoder); __STP(musigProvidor);

	BalanceTransferTransaction* trx = new BalanceTransferTransaction(); __STP(trx);
	// add output utxo
	{
		BalanceUtxo utxo(amount);
		utxo.setAddress(dest);
		trx->addBalanceUtxo(&utxo);
	}

	BalanceUnit totalIn = utxoFinder.getTotalAmount();
	while(collector->hasNext() && totalIn.compareTo(&amount) < 0){
		BalanceUtxo* utxo = collector->next(); __STP(utxo);
		const AddressDescriptor* desc = utxo->getAddress();

		BalanceUtxoReference ref;
		ref.setUtxoId(utxo->getId(), desc);

		IMuSigSigner* signer = musigProvidor->getSigner(desc); __STP(signer);
		Secp256k1Point pt = signer->getxG();
		Secp256k1CompressedPoint xi = Secp256k1CompressedPoint::compress(&pt);
		ref.setXi(&xi);

		trx->addInputUtxoRef(&ref);

		utxoFinder.addUtxo(utxo);

		totalIn = utxoFinder.getTotalAmount();
	}

	ExceptionThrower<BalanceShortageException>::throwExceptionIfCondition(totalIn.compareTo(&amount) < 0, L"Wallet don't have enough balance.", __FILE__, __LINE__);


	// set exchange
	if(totalIn.compareTo(&amount) > 0){
		ChangeAddressStore* changeAddresses = this->account->getChangeAddresses();
		AddressDescriptor* changeDesc = changeAddresses->getNextChangeAddress(encoder); __STP(changeDesc);

		BalanceUnit diff = totalIn - amount;
		BalanceUtxo utxo(diff);
		utxo.setAddress(changeDesc);
		trx->addBalanceUtxo(&utxo);
	}


	// calc fee
	{
		trx->build();
		trx->sign(musigProvidor, &utxoFinder);

		int binSize = trx->binarySize();
		BalanceUnit fee = BalanceUnit(binSize) * feeRate;

		trx->setFeeAmount(&fee);

		AbstractUtxo* u = trx->getUtxo(0);

		BalanceUtxo* utxo = dynamic_cast<BalanceUtxo*>(u);
		utxo->discountFee(fee);
	};

	trx->build();
	trx->sign(musigProvidor, &utxoFinder);

	return __STP_MV(trx);
}

void BalanceTransactionWalletHandler::importTransaction(const AbstractBlockchainTransaction *__trx) {
	const BalanceTransferTransaction* trx = dynamic_cast<const BalanceTransferTransaction*>(__trx);
	WalletAccountTrxRepository* trxRepo = this->account->getWalletAccountTrxRepository();

	bool imported = false;

	// remove used utxos
	{
		const ArrayList<BalanceUtxoReference>* list = trx->getUtxoRefList();
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			BalanceUtxoReference* utxoRef = list->get(i);
			const UtxoId* utxoId = utxoRef->getUtxoId();

			BalanceUtxo* finded = trxRepo->getBalanceUtxo(utxoId); __STP(finded);
			if(finded != nullptr){
				trxRepo->removeUtxo(utxoId);
				imported = true;
			}
		}
	}

	// add utxo
	int maxLoop = trx->getUtxoSize();
	for(int i = 0; i != maxLoop; ++i){
		AbstractUtxo* utxo = trx->getUtxo(i);

		const AddressDescriptor* addressDesc = utxo->getAddress();
		if(this->account->hasAddress(addressDesc)){
			trxRepo->importUtxo(utxo);
			imported = true;
		}
	}

	if(imported){
		trxRepo->importTransaction(trx);
	}
}

} /* namespace codablecash */
