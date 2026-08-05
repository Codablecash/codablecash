/*
 * BlockBody.h
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#ifndef BC_BLOCK_BLOCKBODY_H_
#define BC_BLOCK_BLOCKBODY_H_

#include "base/ArrayList.h"

#include "filestore_block/IBlockObject.h"

using namespace alinous;

namespace codablecash {

class AbstractBlockchainTransaction;
class AbstractBalanceTransaction;
class AbstractControlTransaction;
class AbstractBlockRewordTransaction;
class VoteBlockTransaction;
class AbstractMerkleElement;
class MerkleTree;
class BlockMerkleRoot;
class BlockRewordBase;
class BalanceUnit;
class TransactionId;
class UtxoId;
class MerkleCertificate;
class Abstract32BytesId;
class OmittedBlockBody;
class RevokeMissVotedTicket;
class RevokeMissedTicket;
class AbstractInterChainCommunicationTansaction;
class AbstractSmartcontractTransaction;
class ITransactionVisitor;
class Block;

class BlockBody : public alinous::IBlockObject {
public:
	static const constexpr int MAX_CONTROL_TRX{31250};
	static const constexpr int MAX_ICC_TRX{31250};
	static const constexpr int MAX_BALANCE_TRX{31250};
	static const constexpr int MAX_SMARTCONTRACT_TRX{31250};

	BlockBody();
	explicit BlockBody(uint64_t nonce);
	virtual ~BlockBody();

	void addControlTransaction(const AbstractControlTransaction* trx) noexcept;
	void addInterChainCommunicationTransaction(const AbstractInterChainCommunicationTansaction* trx) noexcept;
	void addBalanceTransaction(const AbstractBalanceTransaction* trx) noexcept;
	void addSmartcontractTransaction(const AbstractSmartcontractTransaction* trx) noexcept;

	void addBlockRewordTransaction(const AbstractBlockRewordTransaction* trx) noexcept;

	void build();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static BlockBody* fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	const BlockMerkleRoot* getMerkleRoot() const noexcept {
		return this->merkleRoot;
	}

	MerkleCertificate* makeCertificate(const Abstract32BytesId* b) const noexcept;
	MerkleCertificate* makeCertificate(const char* hash, int size) const noexcept;

	const ArrayList<AbstractBalanceTransaction>* getBalanceTransactions() const noexcept {
		return this->balanceTransactions;
	}
	const ArrayList<AbstractControlTransaction>* getControlTransactions() const noexcept {
		return this->controlTransactions;
	}
	ArrayList<AbstractInterChainCommunicationTansaction>* getInterChainCommunicationTransactions() const noexcept {
		return this->iccTransactions;
	}
	ArrayList<AbstractSmartcontractTransaction>* getSmartcontractTransactions() const noexcept {
		return this->smartcontractTransactions;
	}

	BalanceUnit getTotalFee() const noexcept;

	const AbstractBalanceTransaction* getBalanceTransaction(const TransactionId* trxId) const noexcept;
	const AbstractBlockchainTransaction* getControlTransaction(const TransactionId* trxId) const noexcept;
	const AbstractInterChainCommunicationTansaction* getInterChainCommunicationTansaction(const TransactionId* trxId) const noexcept;
	const AbstractSmartcontractTransaction* getSmartcontractTransaction(const TransactionId* trxId) const noexcept;


	void setBlockRewordBase(const BlockRewordBase* rewardBase);
	const BlockRewordBase* getBlockRewordBase() const noexcept {
		return this->rewardBase;
	}

	ArrayList<const VoteBlockTransaction>* findVoteBlockTransaction(const UtxoId* utxoId) const noexcept;
	ArrayList<const RevokeMissVotedTicket>* findRevokeMissVotedTicket(const UtxoId* utxoId) const noexcept;
	ArrayList<const RevokeMissedTicket>* findRevokeMissedTicket(const UtxoId* utxoId) const noexcept;


	UnicodeString* toStatString() const noexcept;

	OmittedBlockBody* toOmittedBlockBody() const;

	void visitTransactions(ITransactionVisitor* visitor, const Block* block) const;

#ifdef __DEBUG__
	bool checkTransaction(const AbstractBlockchainTransaction* trx);
	void assertTransactionsBinary();
#endif

private:
	void resetMerkle() noexcept;
	void buildMerleTree();
private:
	MerkleTree* tree;
	BlockMerkleRoot* merkleRoot;

	uint64_t nonce;

	ArrayList<AbstractControlTransaction>* controlTransactions;
	ArrayList<AbstractInterChainCommunicationTansaction>* iccTransactions;
	ArrayList<AbstractBalanceTransaction>* balanceTransactions;
	ArrayList<AbstractSmartcontractTransaction>* smartcontractTransactions;

	BlockRewordBase* rewardBase;
};

} /* namespace codablecash */

#endif /* BC_BLOCK_BLOCKBODY_H_ */
