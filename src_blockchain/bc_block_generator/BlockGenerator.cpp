/*
 * BlockGenerator.cpp
 *
 *  Created on: 2023/03/29
 *      Author: iizuka
 */

#include "bc_block_generator/BlockGenerator.h"
#include "bc_block_generator/BlockRewardCalculator.h"
#include "bc_block_generator/MiningConfig.h"

#include "bc_memorypool/MemoryPool.h"
#include "bc_memorypool/MemPoolTransaction.h"
#include "bc_memorypool/FeeBananceTransactionScanner.h"
#include "bc_memorypool/ControlTransactionScanner.h"

#include "bc_block/Block.h"
#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"

#include "bc_block_vote/VotePart.h"

#include "bc_block_body/BlockBody.h"

#include "base/Exception.h"
#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "bc_trx/AbstractControlTransaction.h"
#include "bc_trx/TransactionId.h"

#include "bc_trx_balance/BalanceTransferTransaction.h"

#include "bc_processor/CentralProcessor.h"

#include "bc_status_cache/BlockchainController.h"

#include "bc_status_cache_context/StatusCacheContext.h"
#include "bc_status_cache_context_finalizer/VotingBlockStatus.h"

#include "bc/ISystemLogger.h"

#include "osenv/funcs.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_status_cache_context_finalizer/VoteCandidate.h"

#include "bc_finalizer_trx/RevokeMissVotedTicket.h"
#include "bc_finalizer_trx/RevokeMissedTicket.h"
#include "bc_finalizer_trx/TicketVotedUtxoReference.h"
#include "bc_finalizer_trx/TicketVotedUtxo.h"
#include "bc_finalizer_trx/VoteBlockTransaction.h"
#include "bc_finalizer_trx/TicketUtxoReference.h"

#include "bc_memorypool/InterChainCommunicationTransactionScanner.h"
#include "bc_memorypool/SmartcontractTansactionScanner.h"

#include "bc_trx/AbstractInterChainCommunicationTansaction.h"

#include "transaction/AbstractSmartcontractTransaction.h"

#include "pow/PoWNonce.h"

#include "numeric/BigInteger.h"

#include "base_timestamp/SystemTimestamp.h"
#include "bc/CodablecashSystemParam.h"

namespace codablecash {

BlockGenerator::BlockGenerator(uint16_t zone, CodablecashSystemParam* config, MemoryPool* memoryPool, BlockchainController* ctrl
		, const MiningConfig *miningConfig, ISystemLogger* logger) {
	this->logger = logger;
	this->config = config;
	this->memoryPool = memoryPool;
	this->ctrl = ctrl;
	this->processor = nullptr;
	this->miningConfig = miningConfig != nullptr ? new MiningConfig(*miningConfig) : nullptr;
	this->zone = zone;
}

BlockGenerator::~BlockGenerator() {
	this->config = nullptr;
	this->memoryPool = nullptr;
	this->ctrl = nullptr;
	this->processor = nullptr;
	delete this->miningConfig;
}

void BlockGenerator::nonceCalculated(uint64_t lastBlockHeight, const BlockHeaderId *lastBlockId, const PoWNonce *nonce) {
	UnicodeString dmsg(L"processing nonceCalculated.");
	this->logger->debugLog(ISystemLogger::DEBUG_POW_CALC_THREAD, &dmsg, __FILE__, __LINE__);


	MemPoolTransaction* memTrx = this->memoryPool->begin(); __STP(memTrx); // mempol locking

	Block* block = this->ctrl->fetechScheduledBlock(); __STP(block);
	if(block != nullptr){
		BlockHeader* header = block->getHeader();
		header->setPoWNonce(nonce);

		SystemTimestamp tm;
		header->setNonceGeneratedTimestamp(&tm);

		// request add block
		this->processor->onMinedBlock(block);
	}
}

Block* BlockGenerator::generateBlock(uint64_t lastBlockHeight,	const BlockHeaderId *lastBlockId, MemPoolTransaction *memTrx) {
	UnicodeString message(L"  [Block Generator] ");
	{
		UnicodeString* bidstr = lastBlockId->toString(); __STP(bidstr);

		message.append(L"Last Height :").append((int64_t)lastBlockHeight);
		message.append(L" Last Block Id : ").append(bidstr);
	}

	uint16_t zoneSelf = this->ctrl->getZoneSelf();

	Block* block = new Block(zoneSelf, lastBlockHeight + 1); __STP(block);
	// block.setPoWResult(lastBlockId, nonce); // bid is last block's id
	block->setLastHeaderId(lastBlockId);
	block->setPoWNonce(&PoWNonce::MAX_NONCE);

	{
		this->ctrl->importCosumedMemTransactions(zoneSelf, memTrx, lastBlockHeight, lastBlockId);

		// import into Block from memory pool
		importTransactions2Block(memTrx, block);

		block->build();

#ifdef __DEBUG__
		{
			bool bl = block->checkMerkleRoot();
			assert(bl == true);

			BlockBody* body = block->getBody();
			body->assertTransactionsBinary();
		}
#endif

		{
			UnicodeString* bidstr = block->getHeader()->getId()->toString(); __STP(bidstr);
			message.append(L"\n");
			message.append(L"        --> Height: ").append((int64_t)block->getHeight());
			message.append(L" Header Id: ").append(bidstr);

			BlockBody* body = block->getBody();
			UnicodeString* bodtstat = body->toStatString(); __STP(bodtstat);
			message.append(L" ").append(bodtstat);

			this->logger->log(&message);
		}
	}

	return __STP_MV(block);
}

void BlockGenerator::importTransactions2Block(MemPoolTransaction *memTrx, Block *block) {
	const BlockHeader* header = block->getHeader();
	const BlockHeaderId* lastheaderId = header->getLastHeaderId();
	uint64_t lastheight = header->getHeight() - 1;
	uint16_t zone = header->getZone();

	IStatusCacheContext* context = this->ctrl->getStatusCacheContext(zone, lastheaderId, lastheight); __STP(context);

	// control transaction
	importControlTransactions2Block(memTrx, block, header, context);

	// inter chain communication transaction
	importInterChainCommunicationTransactions2Block(memTrx, block, header, context);

	// balance transaction
	importBalanceTransactions2Block(memTrx, block, header, context);

	// smart contract
	importSmartcontractTransactions2Block(memTrx, block, header, context);

	// buildRewordBase
	buildRewordBase(memTrx, context, block);
}

void BlockGenerator::importControlTransactions2Block(MemPoolTransaction* memTrx, Block* block
		, const BlockHeader* header, IStatusCacheContext* context) {
	// control transaction
	const BlockHeaderId* lastHeaderId = header->getLastHeaderId();

	const BlockBody* blockBody = block->getBody();

	/**
	 * ControlTransactionScanner eliminates consumed transactions
	 */
	ControlTransactionScanner* scanner = memTrx->beginScanControlTransaction(); __STP(scanner);
	while(scanner->hasNext()){
		AbstractControlTransaction* trx = scanner->next(); __STP(trx);
		const TransactionId* trxId = trx->getTransactionId();

		// Voting timing protection
		VoteBlockTransaction* voteTrx = dynamic_cast<VoteBlockTransaction*>(trx);
		if(voteTrx != nullptr){
			TrxValidationResult result = trx->validateFinal(header, memTrx, context);

			if(result == TrxValidationResult::OK){
				block->addControlTransaction(trx);
				context->importControlTransaction(header, blockBody, trx, this->logger);
			}
			continue;
		}

		TrxValidationResult result = trx->validateFinal(header, memTrx, context);
		if(result == TrxValidationResult::OK){
			block->addControlTransaction(trx);
			context->importControlTransaction(header, blockBody, trx, this->logger);
		}
	}
}

void BlockGenerator::importInterChainCommunicationTransactions2Block(MemPoolTransaction *memTrx
		, Block *block, const BlockHeader *header, IStatusCacheContext *context) {

	InterChainCommunicationTransactionScanner* scanner = memTrx->beginScanInterChainCommunicationTransaction(); __STP(scanner);
	while(scanner->hasNext()){
		AbstractInterChainCommunicationTansaction* trx = scanner->next(); __STP(trx);
		const TransactionId* trxId = trx->getTransactionId();

		TrxValidationResult result = trx->validateFinal(header, memTrx, context);

		if(result == TrxValidationResult::OK){
			block->addInterChainCommunicationTransaction(trx);
			context->importInterChainCommunicationTransaction(header, trx, this->logger);
		}
	}
}

void BlockGenerator::importSmartcontractTransactions2Block(	MemPoolTransaction *memTrx
		, Block *block, const BlockHeader *header, IStatusCacheContext *context) {

	SmartcontractTansactionScanner* scanner = memTrx->beginScanSmartcontractTansaction(); __STP(scanner);
	while(scanner->hasNext()){
		AbstractSmartcontractTransaction* trx = scanner->next(); __STP(trx);
		const TransactionId* trxId = trx->getTransactionId();

		TrxValidationResult result = trx->validateFinal(header, memTrx, context);
		if(result == TrxValidationResult::OK){
			block->addSmartcontractTransaction(trx);
			context->importSmartcontractTransaction(header, trx, this->logger);
		}
	}
}

void BlockGenerator::buildRewordBase(MemPoolTransaction *memTrx, IStatusCacheContext *context, Block *block) {
	uint16_t zone = context->getZone();
	uint64_t height = block->getHeight();
	BlockHeader* header = block->getHeader();
	BlockBody* body = block->getBody();

	uint16_t numZones = context->getNumZones(height);

	BlockRewardCalculator calc(numZones);

	// set fee
	calc.setFee(body->getTotalFee());

	// pow
	calc.importPowConfig(this->miningConfig);

	// get correct header Id and vote
	VotingBlockStatus* status = context->getVotingBlockStatus(header); __STP(status);
	if(status != nullptr && !status->isEmpty()){
		CodablecashBlockchain* blockchain = context->getBlockChain();
		BlockHeaderStoreManager* headerManger = blockchain->getHeaderManager(zone);

		uint64_t votedHeight = status->getHeight();
		const BlockHeaderId* headerIdOnVoting = status->getBlockHeaderId();
		int voteBeforeNBlocks = this->config->getVoteBeforeNBlocks(votedHeight);

		// correct header Id
		BlockHeader* votedHeader = headerManger->getNBlocksBefore(headerIdOnVoting, votedHeight, voteBeforeNBlocks); __STP(votedHeader);
		const BlockHeaderId* votedHeaderId = votedHeader->getId();

		const VotePart* vpart = header->getVotePart();
		const VotedHeaderIdGroup* group = vpart->getVotedGroup(votedHeaderId);
		if(group != nullptr){
			calc.importVotes(group, status, body);
		}
	}

	calc.calcRewords(height, zone);

	calc.exportCoinBaseTransaction(body, height);

	uint64_t ticketVoterFeeBasisPoint = this->config->getTicketVoterFeeBasisPoint(height);
	calc.exportStakeBaseTransactions(body, ticketVoterFeeBasisPoint, height);


	// revoke
	int votePerBlock = this->config->getVotePerBlock();
	if(!block->isFinalizing(votePerBlock) && status != nullptr && !status->isEmpty()){
		removkeUnvotedTickets(memTrx, context, block, status);
	}
}

void BlockGenerator::removkeUnvotedTickets(MemPoolTransaction *memTrx, IStatusCacheContext *context, Block *block, VotingBlockStatus* status) {
	uint16_t zone = context->getZone();
	uint64_t height = block->getHeight();
	BlockHeader* header = block->getHeader();
	BlockBody* body = block->getBody();

	CodablecashBlockchain* blockchain = context->getBlockChain();
	BlockHeaderStoreManager* headerManger = blockchain->getHeaderManager(zone);

	// vote timing
	uint64_t votedHeight = status->getHeight();
	const BlockHeaderId* headerIdOnVoting = status->getBlockHeaderId();
	int voteBeforeNBlocks = this->config->getVoteBeforeNBlocks(votedHeight);

	BlockHeader* votedHeader = headerManger->getNBlocksBefore(headerIdOnVoting, votedHeight, voteBeforeNBlocks); __STP(votedHeader);
	const BlockHeaderId* votedHeaderId = votedHeader->getId();

	const VotePart* vpart = header->getVotePart();
	const VotedHeaderIdGroup* group = vpart->getVotedGroup(votedHeaderId);

	const ArrayList<VoteCandidate>* list = status->getCandidatesList();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		VoteCandidate* candidate = list->get(i);

		if(!candidate->isVoted()){
			revokeUnvokedcandidate(memTrx, context, block, candidate);
		}
	}
}

void BlockGenerator::revokeUnvokedcandidate(MemPoolTransaction *memTrx, IStatusCacheContext *context, Block *block, VoteCandidate *candidate) {
	const UtxoId* utxoId = candidate->getUtxoId();

	BlockBody* body = block->getBody();

	// if voted wrong
	{
		ArrayList<const VoteBlockTransaction>* trxlist = body->findVoteBlockTransaction(utxoId); __STP(trxlist);
		if(!trxlist->isEmpty()){
			const VoteBlockTransaction* trx = trxlist->get(0);
			addRevokeMissVotedTicket(memTrx, context, block, candidate, trx);
			return;
		}
	}

	// if not voted
	addRevokeMissedTicket(memTrx, context, block, candidate);
}

/**
 * Voted wrong block
 * @param memTrx
 * @param context
 * @param block
 * @param candidate
 * @param trx
 */
void BlockGenerator::addRevokeMissVotedTicket(MemPoolTransaction *memTrx, IStatusCacheContext *context,	Block *block, VoteCandidate *candidate, const VoteBlockTransaction* trx) {
	RevokeMissVotedTicket* revokeTrx = new RevokeMissVotedTicket(); __STP(revokeTrx);

	{
		TicketVotedUtxoReference ref;
		const TicketVotedUtxo* votedUtxo = trx->getTicketVotedUtxo();
		const UtxoId* utxoId = votedUtxo->getId();

		const AddressDescriptor* desc = candidate->getAddressDescriptor();
		ref.setUtxoId(utxoId, desc);

		revokeTrx->setTicketVotedUtxoReference(&ref);
	}

	{
		BalanceUnit ticketPrice = candidate->getTicletPrice();
		const AddressDescriptor* desc = candidate->getAddressDescriptor();

		BalanceUtxo utxo(ticketPrice);
		utxo.setAddress(desc);

		revokeTrx->addBalanceUtxo(&utxo);
	}

	revokeTrx->build();

#ifdef __DEBUG__
	{
		BlockHeader* header = block->getHeader();
		TrxValidationResult result = revokeTrx->validateFinal(header, memTrx, context);
		assert(result == TrxValidationResult::OK);

		assert(revokeTrx->canAddMempool() == false);
	}
#endif

	block->addControlTransaction(revokeTrx);
}

void BlockGenerator::addRevokeMissedTicket(MemPoolTransaction *memTrx, IStatusCacheContext *context, Block *block, VoteCandidate *candidate) {
	RevokeMissedTicket* revokeTrx = new RevokeMissedTicket(); __STP(revokeTrx);

	{
		const UtxoId* utxoId = candidate->getUtxoId();
		TicketUtxoReference ref;

		const AddressDescriptor* desc = candidate->getAddressDescriptor();
		ref.setUtxoId(utxoId, desc);

		revokeTrx->setTicketUtxoReference(&ref);
	}

	{
		BalanceUnit ticketPrice = candidate->getTicletPrice();
		const AddressDescriptor* desc = candidate->getAddressDescriptor();

		BalanceUtxo utxo(ticketPrice);
		utxo.setAddress(desc);

		revokeTrx->addBalanceUtxo(&utxo);
	}

	revokeTrx->build();

#ifdef __DEBUG__
	{
		BlockHeader* header = block->getHeader();
		TrxValidationResult result = revokeTrx->validateFinal(header, memTrx, context);
		assert(result == TrxValidationResult::OK);
	}
#endif

	block->addControlTransaction(revokeTrx);
}

void BlockGenerator::importBalanceTransactions2Block(MemPoolTransaction *memTrx,
		Block *block, const BlockHeader *header, IStatusCacheContext *context) {
	/**
	 * FeeBananceTransactionScanner eliminates consumed transactions
	 */
	FeeBananceTransactionScanner* scanner = memTrx->beginScanBalanceTransaction(); __STP(scanner);
	while(scanner->hasNext()){
		AbstractBalanceTransaction* trx = scanner->next(); __STP(trx);
		const TransactionId* trxId = trx->getTransactionId();

		TrxValidationResult result = trx->validateFinal(header, memTrx, context);

		if(result == TrxValidationResult::OK){
			block->addBalanceTransaction(trx);
			context->importBalanceTransaction(header, trx, this->logger);
		}
	}
}

BlockHeader* BlockGenerator::getTopBlockHeader() const {
	return this->ctrl->getTopHeader();
}

bool BlockGenerator::continueMining() const {
	return !this->ctrl->checkAcceptSecondRealBlockOnMining();
}

BlockHeader* BlockGenerator::changeMiningTarget() {
	return this->ctrl->changeMiningTarget();
}

BigInteger BlockGenerator::calcTargetDiff(uint64_t lastHeight, const BlockHeaderId *lastHeaderId, const SystemTimestamp* tm) {
	return this->ctrl->calcTargetDifficulty(this->zone, lastHeight, lastHeaderId, tm);
}

void BlockGenerator::setScheduledBlock(const Block *block) {
	this->ctrl->setScheduledBlock(block);
}

} /* namespace codablecash */
