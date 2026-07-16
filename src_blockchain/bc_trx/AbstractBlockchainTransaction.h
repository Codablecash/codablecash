/*
 * AbstractBlockchainTransaction.h
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#ifndef BC_TRX_ABSTRACTBLOCKCHAINTRANSACTION_H_
#define BC_TRX_ABSTRACTBLOCKCHAINTRANSACTION_H_

#include "filestore_block/IBlockObject.h"

namespace alinous {
class SystemTimestamp;
};
using namespace alinous;

namespace codablecash {

class TransactionId;
class AbstractUtxo;
class AbstractUtxoReference;
class BlockHeader;
class IStatusCacheContext;
class MemPoolTransaction;
class BalanceUnit;
class BloomFilter1024;
class AddressDescriptor;
class IUtxoRefChecker;
class IAddressChecker;
class MerkleTree;
class UtxoId;
class TransactionVersion;

enum class TrxValidationResult
{
	OK,
	PENDING,
	INVALID
};

enum class UtxoValidationResult
{
	INVALID, // 0
	ON_MEMORY, // 1
	ON_CHAIN // 2
};

class AbstractBlockchainTransaction : public alinous::IBlockObject {
public:
	static const constexpr uint8_t TRX_TYPE_GENESIS{1};
	static const constexpr uint8_t TRX_TYPE_BANANCE_TRANSFER{2};

	static const constexpr uint8_t TRX_TYPE_REGISTER_VOTE_POOL{3};
	static const constexpr uint8_t TRX_TYPE_REGISTER_TICKET{4};
	static const constexpr uint8_t TRX_TYPE_VOTE_BLOCK{5};
	static const constexpr uint8_t TRX_TYPE_REVOKE_MISSED_TICKET{6};
	static const constexpr uint8_t TRX_TYPE_REVOKE_MISS_VOTED_TICKET{7};

	static const constexpr uint8_t TRX_TYPE_COIN_BASE{8};
	static const constexpr uint8_t TRX_TYPE_STAKE_BASE{9};

	static const constexpr uint8_t TRX_TYPE_SMARTCONTRACT_NOP{10};

	static const constexpr uint8_t TRX_TYPE_ICC_NOP{20};
	static const constexpr uint8_t TRX_TYPE_ICC_ZONE_EXTEND_REQUESTED{21};


	AbstractBlockchainTransaction(const AbstractBlockchainTransaction& inst);
	AbstractBlockchainTransaction();
	virtual ~AbstractBlockchainTransaction();

	static AbstractBlockchainTransaction* createFromBinary(ByteBuffer* in);

	virtual uint8_t getType() const noexcept = 0;

	virtual void fromBinary(ByteBuffer* in) = 0;

	virtual void build() = 0;

	const TransactionId* getTransactionId() const noexcept {
		return this->trxId;
	}

	virtual BalanceUnit getFee() const noexcept = 0;
	virtual BalanceUnit getFeeRate() const noexcept = 0;

	virtual int getUtxoSize() const noexcept = 0;
	virtual AbstractUtxo* getUtxo(int i) const noexcept = 0;
	virtual int getUtxoReferenceSize() const noexcept = 0;
	virtual AbstractUtxoReference* getUtxoReference(int i) const noexcept = 0;

	virtual bool validateOnAccept(MemPoolTransaction *memTrx, IStatusCacheContext* context) const = 0;
	virtual TrxValidationResult validateFinal(const BlockHeader* header, MemPoolTransaction *memTrx, IStatusCacheContext* context) const = 0;
	virtual TrxValidationResult validateReported(const BlockHeader* header, IStatusCacheContext* context) const;


	virtual bool checkFilter(const ArrayList<BloomFilter1024> *filtersList) const;
	virtual bool checkFilteredUxtoRef(const IUtxoRefChecker* utxoRefChecker) const;
	virtual bool checkFilteredAddress(const IAddressChecker* addressChecker) const;

	bool checkUtxoRefs() const noexcept;

	/**
	 * Revoke, reward transactions are false
	 */
	virtual bool canAddMempool() const noexcept {
		return true;
	}

	/**
	 * To add Merkle tree elements inside the transaction.
	 * @param tree
	 */
	virtual void addInternalMerkleTreeElement(MerkleTree* tree){};

	const SystemTimestamp* getTimestamp() const noexcept {
		return this->timestamp;
	}


protected:
	virtual UtxoValidationResult validateUtxos(MemPoolTransaction *memTrx, IStatusCacheContext* context, const BalanceUnit& fee) const;

private:
	bool hasUsedUtxo(ArrayList<const UtxoId>* usedUtxo, const UtxoId* utxoId) const;

	bool checkFilters(const ArrayList<BloomFilter1024> *filtersList, const AddressDescriptor* desc) const;

protected:
	TransactionId* trxId;
	SystemTimestamp* timestamp;
	TransactionVersion* version;
};

} /* namespace codablecash */

#endif /* BC_TRX_ABSTRACTBLOCKCHAINTRANSACTION_H_ */
