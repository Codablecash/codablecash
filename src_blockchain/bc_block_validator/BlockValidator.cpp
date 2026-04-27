/*
 * BlockValidator.cpp
 *
 *  Created on: 2023/11/27
 *      Author: iizuka
 */

#include "bc_block_validator/BlockValidator.h"

#include "bc_block/Block.h"
#include "bc_block/BlockHeader.h"

#include "bc_block_body/BlockBody.h"
#include "bc_block_body/CoinbaseTransaction.h"
#include "bc_block_body/BlockRewordBase.h"
#include "bc_block_body/StakeBaseTransaction.h"
#include "bc_block_body/Stakebase.h"

#include "base/StackRelease.h"
#include "bc/CodablecashSystemParam.h"

#include "pow/PoWNonce.h"
#include "pow/PoWNonceResult.h"

#include "numeric/BigInteger.h"

#include "bc/ExceptionThrower.h"
#include "bc_block_generator/BlockRewardCalculator.h"

#include "bc_block_validator/BlockValidationException.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc_status_cache_context_finalizer/VotingBlockStatus.h"

#include "bc_memorypool/MemoryPool.h"
#include "bc_memorypool/MemPoolTransaction.h"

#include "bc_trx/AbstractBlockchainTransaction.h"
#include "bc_trx/AbstractControlTransaction.h"
#include "bc_trx/AbstractBalanceTransaction.h"
#include "bc_trx/AbstractUtxo.h"
#include "bc_trx/UtxoId.h"
#include "bc_trx/AbstractInterChainCommunicationTansaction.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_block/BlockHeaderId.h"

#include "bc_block_vote/VotePart.h"
#include "bc_block_vote/VotedHeaderIdGroup.h"

#include "bc_status_cache_context_finalizer/VoteCandidate.h"

#include "bc_finalizer_trx/VoteBlockTransaction.h"
#include "bc_finalizer_trx/RevokeMissVotedTicket.h"
#include "bc_finalizer_trx/RevokeMissedTicket.h"
#include "bc_finalizer_trx/TicketVotedUtxoReference.h"
#include "bc_finalizer_trx/TicketVotedUtxo.h"

#include "bc_trx_balance/BalanceUtxo.h"

#include "bc_block_generator/BlockRewardStakeBase.h"

#include "bc_base/AddressDescriptor.h"

#include "transaction/AbstractSmartcontractTransaction.h"

#include "base_timestamp/SystemTimestamp.h"

namespace codablecash {

BlockValidator::BlockValidator(const Block* block, CodablecashSystemParam* config, MemoryPool* memoryPool, BlockchainController* ctrl) {
	this->block = block;
	this->config = config;
	this->memoryPool = memoryPool;
	this->ctrl = ctrl;
}

BlockValidator::~BlockValidator() {
	this->block = nullptr;
	this->config = nullptr;
	this->memoryPool = nullptr;
	this->ctrl = nullptr;
}

void BlockValidator::validate() {
	BlockHeader* header = this->block->getHeader();
	BlockBody* body = this->block->getBody();

	// validate last header
	validateLastHeader();

	validateHashrate();

	{
		uint16_t zoneSelf = this->ctrl->getZoneSelf();
		const BlockHeaderId *bid = header->getLastHeaderId();
		uint64_t lastBlockHeight = header->getHeight() - 1;

		MemPoolTransaction* memTrx = this->memoryPool->begin(); __STP(memTrx);
		// setup mempool transaction
		this->ctrl->importCosumedMemTransactions(zoneSelf, memTrx, lastBlockHeight, bid);

		validateTransactionsInBlock(memTrx);
	}
}

void BlockValidator::validateLastHeader() {
	if(this->block->getHeight() == 1){
		return;
	}
	BlockHeader* header = this->block->getHeader();

	uint16_t zone = this->ctrl->getZoneSelf();
	const BlockHeaderId *lastHeaderId = header->getLastHeaderId();
	uint64_t lastBlockHeight = header->getHeight() - 1;

	Block* lastBlock = this->ctrl->getBlockHeightAt(zone, lastBlockHeight, lastHeaderId); __STP(lastBlock);
	ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(lastBlock == nullptr
			, L"Invalid last block id.", __FILE__, __LINE__);

	const SystemTimestamp* nonceTm = header->getNonceGeneratedtimestamp();
	const SystemTimestamp* tm = header->getTimestamp();
	ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(tm->compareTo(nonceTm) > 0
			, L"The block Timestamp must be before nonce calculated time.", __FILE__, __LINE__);

}

void BlockValidator::validateTransactionsInBlock(MemPoolTransaction *memTrx) {
	const BlockHeader* header = this->block->getHeader();

	const BlockHeaderId* lastheaderId = header->getLastHeaderId();
	uint64_t lastheight = header->getHeight() - 1;
	uint16_t zone = header->getZone();

	IStatusCacheContext* context = this->ctrl->getStatusCacheContext(zone, lastheaderId, lastheight); __STP(context);

	validateControlTransactions(memTrx, header, context);
	validateInterChainCommunicationTransactions(memTrx, header, context);
	validateBalanceTransactions(memTrx, header, context);
	validateSmartcontractTransactions(memTrx, header, context);

	validateRewordBase(memTrx, context);
}


void BlockValidator::validateControlTransactions(MemPoolTransaction *memTrx, const BlockHeader *header, IStatusCacheContext *context) {
	BlockBody* body = this->block->getBody();
	const BlockHeaderId* lastHeaderId = header->getLastHeaderId();

	const ArrayList<AbstractControlTransaction>* list = body->getControlTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const AbstractControlTransaction* trx = list->get(i);

		TrxValidationResult result = trx->validateReported(header, context);
		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(result != TrxValidationResult::OK
				, L"Invalid transaction.", __FILE__, __LINE__);

		context->importControlTransaction(header, body, trx, this->ctrl->getLogger());
	}
}

void BlockValidator::validateBalanceTransactions(MemPoolTransaction *memTrx, const BlockHeader *header, IStatusCacheContext *context) {
	BlockBody* body = this->block->getBody();

	const ArrayList<AbstractBalanceTransaction>* list = body->getBalanceTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const AbstractBalanceTransaction* trx = list->get(i);

		TrxValidationResult result = trx->validateReported(header, context);
		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(result != TrxValidationResult::OK
				, L"Invalid transaction.", __FILE__, __LINE__);

		context->importBalanceTransaction(header, trx, this->ctrl->getLogger());
	}
}

void BlockValidator::validateInterChainCommunicationTransactions(MemPoolTransaction *memTrx, const BlockHeader *header, IStatusCacheContext *context) {
	BlockBody* body = this->block->getBody();

	const ArrayList<AbstractInterChainCommunicationTansaction>* list = body->getInterChainCommunicationTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const AbstractInterChainCommunicationTansaction* trx = list->get(i);

		TrxValidationResult result = trx->validateReported(header, context);
		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(result != TrxValidationResult::OK
				, L"Invalid transaction.", __FILE__, __LINE__);

		context->importInterChainCommunicationTransaction(header, trx, this->ctrl->getLogger());
	}
}

void BlockValidator::validateSmartcontractTransactions(MemPoolTransaction *memTrx, const BlockHeader *header, IStatusCacheContext *context) {
	BlockBody* body = this->block->getBody();

	const ArrayList<AbstractSmartcontractTransaction>* list = body->getSmartcontractTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const AbstractSmartcontractTransaction* trx = list->get(i);

		TrxValidationResult result = trx->validateReported(header, context);
		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(result != TrxValidationResult::OK
				, L"Invalid transaction.", __FILE__, __LINE__);

		context->importSmartcontractTransaction(header, trx, this->ctrl->getLogger());
	}
}

void BlockValidator::validateHashrate() {
	if(this->block->getHeight() == 1){
		return;
	}

	BlockHeader* header = this->block->getHeader();

	const PoWNonce* nonce = header->getPoWNonce();
	PoWNonceResult* res = nonce->calcResult(header->getLastHeaderId(), header->getMerkleRoot(), header->getTimestamp()); __STP(res);
	BigInteger diff = res->getDifficulty();


	uint16_t zone = header->getZone();
	uint64_t lastHeight = header->getHeight() - 1;
	const BlockHeaderId *lastHeaderId = header->getLastHeaderId();
	const SystemTimestamp* tm = header->getTimestamp();

	BigInteger targetDiff = this->ctrl->calcTargetDifficulty(zone, lastHeight, lastHeaderId, tm);

	ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(diff.compareTo(targetDiff) < 0
			, L"Difficulty of the hash is not enough.", __FILE__, __LINE__);
}

void BlockValidator::validateRewordBase(MemPoolTransaction *memTrx, IStatusCacheContext *context) {
	if(this->block->getHeight() == 1){
		return;
	}

	uint16_t zone = context->getZone();
	uint64_t height = this->block->getHeight();
	BlockHeader* header = this->block->getHeader();
	BlockBody* body = this->block->getBody();
	const BlockRewordBase* rewardBase =  body->getBlockRewordBase();

	uint16_t numZones = context->getNumZones(height);

	BlockRewardCalculator calc(numZones);
	// set fee
	calc.setFee(body->getTotalFee());

	// pow
	{
		const CoinbaseTransaction* coinbaseTrx = rewardBase->getCoinbaseTransaction();
		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(coinbaseTrx == nullptr
				, L"Coinbase does not exists.", __FILE__, __LINE__);

		TrxValidationResult result = coinbaseTrx->validateReported(header, context);
		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(result != TrxValidationResult::OK || height != coinbaseTrx->getHeight()
				, L"Invalid Coinbase transaction.", __FILE__, __LINE__);

		int utxoSize = coinbaseTrx->getUtxoSize();
		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(utxoSize != 1
				, L"Coinbase output UTXO is wrong.", __FILE__, __LINE__);

		AbstractUtxo* utxo = coinbaseTrx->getUtxo(0);
		const AddressDescriptor* powAddress = utxo->getAddress();

		calc.setPoWConfig(powAddress);
	}

	// get correct header Id and vote
	VotingBlockStatus* status = context->getVotingBlockStatus(header); __STP(status);
	BlockHeader* votedHeader = nullptr;

	if(status != nullptr && !status->isEmpty()){
		CodablecashBlockchain* blockchain = context->getBlockChain();
		BlockHeaderStoreManager* headerManger = blockchain->getHeaderManager(zone);

		uint64_t votedHeight = status->getHeight();
		const BlockHeaderId* headerIdOnVoting = status->getBlockHeaderId();
		int voteBeforeNBlocks = this->config->getVoteBeforeNBlocks(votedHeight);

		// correct header Id
		votedHeader = headerManger->getNBlocksBefore(headerIdOnVoting, votedHeight, voteBeforeNBlocks); __STP(votedHeader);
		const BlockHeaderId* votedHeaderId = votedHeader->getId();

		const VotePart* vpart = header->getVotePart();
		const VotedHeaderIdGroup* group = vpart->getVotedGroup(votedHeaderId);
		if(group != nullptr){
			calc.importVotes(group, status, body);
		}

		__STP_MV(votedHeader);
	}
	__STP(votedHeader);

	calc.calcRewords(height, zone);

	// coinbase and stake base
	{
		const CoinbaseTransaction* coinbaseTrx = rewardBase->getCoinbaseTransaction();
		CoinbaseTransaction* calclatedTrx = calc.generateCoinbaseTransaction(height); __STP(calclatedTrx);

		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(coinbaseTrx->coinbaseEquals(calclatedTrx) == false
				, L"Invalid Coinbase reward value.", __FILE__, __LINE__);
	}

	// validate stake reword transctions
	if(votedHeader != nullptr){
		const BlockHeaderId* votedHeaderId = votedHeader->getId();

		const ArrayList<StakeBaseTransaction>* listtrx = rewardBase->getStakeBases();
		const ArrayList<BlockRewardStakeBase>* calclist = calc.getStakeRewardBases();

		ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(listtrx->size() != calclist->size()
				, L"Invalid Stake reward list.", __FILE__, __LINE__);

		uint64_t ticketVoterFeeBasisPoint = this->config->getTicketVoterFeeBasisPoint(height);

		int maxLoop = listtrx->size();
		for(int i = 0; i != maxLoop; ++i){
			StakeBaseTransaction* trx = listtrx->get(i);
			BlockRewardStakeBase* rewardBase = calclist->get(i);

			TrxValidationResult result = trx->validateReported(header, context);
			ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(result != TrxValidationResult::OK || height != trx->getHeight()
					|| trx->getUtxoReferenceSize() != 2 || trx->getUtxoSize() != 2
					, L"Invalid Stakebase transaction.", __FILE__, __LINE__);

			// chech staking reward
			// check stakebase
			{
				const Stakebase* stakebase = trx->getStakebase();
				BalanceUnit totalReword = rewardBase->getReward();

				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(totalReword.compareTo(stakebase->getAmount()) != 0
						, L"Amount of stake reward is wrong.", __FILE__, __LINE__);
			}
			// check utxoref TicketVotedUtxoReference
			{
				TicketVotedUtxoReference* utxoRef = trx->getTicketVotedUtxoReference();
				const UtxoId* u1 = utxoRef->getUtxoId();

				TicketVotedUtxo* utxo = rewardBase->getTicketVotedUtxo();
				const UtxoId* u2 = utxo->getId();

				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(u1->equals(u2) == false
						, L"Wrong utxo id.", __FILE__, __LINE__);

			}
			// check ticket owner reward
			{
				const AbstractUtxo* utxo = trx->getUtxo(0);
				const BalanceUtxo* butxo = dynamic_cast<const BalanceUtxo*>(utxo);
				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(butxo == nullptr
						, L"Invalid utxo @0.", __FILE__, __LINE__);

				const AddressDescriptor* address = rewardBase->getAddressDescriptor();
				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(address->compareTo(butxo->getAddress()) != 0
						, L"Invalid address of the ticket owner.", __FILE__, __LINE__);


				BalanceUnit amount = rewardBase->calcTicketOwnerBalance(ticketVoterFeeBasisPoint);
				BalanceUnit utxoAmount = butxo->getAmount();

				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(amount.compareTo(&utxoAmount) != 0
						, L"Reward amount to Ticket owner is wrong.", __FILE__, __LINE__);
			}
			// check ticket pool reward
			{
				const AbstractUtxo* utxo = trx->getUtxo(1);
				const BalanceUtxo* butxo = dynamic_cast<const BalanceUtxo*>(utxo);
				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(butxo == nullptr
						, L"Invalid utxo @1.", __FILE__, __LINE__);

				const AddressDescriptor* address = rewardBase->getVoterAddressDescriptor();
				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(address->compareTo(butxo->getAddress()) != 0
						, L"Invalid address of the ticket voter.", __FILE__, __LINE__);

				BalanceUnit amount = rewardBase->calcVoterFeeBalance(ticketVoterFeeBasisPoint);
				BalanceUnit utxoAmount = butxo->getAmount();

				ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(amount.compareTo(&utxoAmount) != 0
						, L"Reward amount to Ticket Voter is wrong.", __FILE__, __LINE__);
			}
		}
	}

	// check revoke transactions
	int votePerBlock = this->config->getVotePerBlock();
	if(!block->isFinalizing(votePerBlock) && status != nullptr && !status->isEmpty()){
		validateUnvotedTickets(memTrx, context, status);
	}
}

void BlockValidator::validateUnvotedTickets(MemPoolTransaction *memTrx, IStatusCacheContext *context, VotingBlockStatus *status) {
	uint16_t zone = context->getZone();
	uint64_t height = this->block->getHeight();
	BlockHeader* header = this->block->getHeader();
	BlockBody* body = this->block->getBody();

	CodablecashBlockchain* blockchain = context->getBlockChain();
	BlockHeaderStoreManager* headerManger = blockchain->getHeaderManager(zone);

	// status is on vote timing, calculate VoteBeforeNBlocks before the voteing height
	uint64_t votedHeight = status->getHeight();
	const BlockHeaderId* headerIdOnVoting = status->getBlockHeaderId();
	int voteBeforeNBlocks = this->config->getVoteBeforeNBlocks(votedHeight);

	BlockHeader* correctVotedHeader = headerManger->getNBlocksBefore(headerIdOnVoting, votedHeight, voteBeforeNBlocks); __STP(correctVotedHeader);
	const BlockHeaderId* correctVotedHeaderId = correctVotedHeader->getId();

	const VotePart* vpart = header->getVotePart();
	const VotedHeaderIdGroup* group = vpart->getVotedGroup(correctVotedHeaderId);

	const ArrayList<VoteCandidate>* list = status->getCandidatesList();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		VoteCandidate* candidate = list->get(i);

		if(!candidate->isVoted()){
			validateUnvokedcandidate(memTrx, context, candidate, correctVotedHeaderId);
		}
	}
}

void BlockValidator::validateUnvokedcandidate(MemPoolTransaction *memTrx, IStatusCacheContext *context, VoteCandidate *candidate, const BlockHeaderId* correctVotedHeaderId) {
	const UtxoId* utxoId = candidate->getUtxoId();
	BlockBody* body = this->block->getBody();

	// if voted wrong
	{
		ArrayList<const VoteBlockTransaction>* trxlist = body->findVoteBlockTransaction(utxoId); __STP(trxlist);
		if(!trxlist->isEmpty()){
			// more than 1 transactions are there
			int num = trxlist->size();
			ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(num != 1
					, L"Too many vote for same trx is there.", __FILE__, __LINE__);

			// check trx has wrong vote header id
			const VoteBlockTransaction* trx = trxlist->get(0);
			BlockHeaderId* trxVotedHeaderId = trx->getVotedHeaderId();
			ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(trxVotedHeaderId->equals(correctVotedHeaderId)
					, L"Correct vote is not counted.", __FILE__, __LINE__);

			// check miss-voted revoke transaction
			const AbstractUtxo* votedUtxo = trx->getUtxo(0);
			const UtxoId* votedUtxoId = votedUtxo->getId();
			ArrayList<const RevokeMissVotedTicket>* missvotelist = body->findRevokeMissVotedTicket(votedUtxoId); __STP(missvotelist);

			// check transaction number for the utxoid is 1
			num = missvotelist->size();
			ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(num != 1
					, L"Miss voted ticket is not revoked.", __FILE__, __LINE__);
			return;
		}
	}

	// if not voted check missed revoke transaction
	ArrayList<const RevokeMissedTicket>* misslist = body->findRevokeMissedTicket(utxoId); __STP(misslist);

	// check transaction number for the utxoid is 1
	int num = misslist->size();
	ExceptionThrower<BlockValidationException>::throwExceptionIfCondition(num != 1
			, L"Missed ticket is not revoked.", __FILE__, __LINE__);
}

} /* namespace codablecash */
