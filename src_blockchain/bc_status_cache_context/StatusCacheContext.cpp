/*
 * StatusCacheContext.cpp
 *
 *  Created on: 2023/04/23
 *      Author: iizuka
 */

#include <cstdio>

#include "bc_status_cache_context/StatusCacheContext.h"
#include "bc_status_cache_context/TransactionContextCache.h"
#include "bc_status_cache_context/UtxoCacheContext.h"
#include "bc_status_cache_context/NullLockinManager.h"
#include "bc_status_cache_context/RemoteUtxoDetector.h"

#include "bc_status_cache_context_finalizer/VoterStatusCacheContext.h"

#include "base/UnicodeString.h"
#include "base/ArrayList.h"

#include "base_io/File.h"

#include "base_thread/ConcurrentGate.h"

#include "bc_block_body/BlockBody.h"

#include "bc_block/BlockHeader.h"

#include "bc_trx/AbstractBalanceTransaction.h"
#include "bc_trx/AbstractControlTransaction.h"
#include "bc_trx/AbstractUtxoReference.h"
#include "bc_trx/AbstractUtxo.h"
#include "bc_trx/AbstractBlockchainTransaction.h"
#include "bc_trx/AbstractInterChainCommunicationTansaction.h"

#include "bc_finalizer/VoterEntry.h"
#include "bc_finalizer/VoteTicket.h"

#include "bc_finalizer_trx/RegisterVotePoolTransaction.h"
#include "bc_finalizer_trx/RegisterTicketTransaction.h"
#include "bc_finalizer_trx/TicketUtxo.h"
#include "bc_finalizer_trx/VoteBlockTransaction.h"
#include "bc_finalizer_trx/TicketUtxoReference.h"

#include "base/StackRelease.h"

#include "bc/CodablecashSystemParam.h"

#include "bc_finalizer/TicketVoteSelector.h"

#include "bc_status_cache_context_finalizer/VoterStatusMappedCacheContext.h"
#include "bc_status_cache_context_finalizer/VotingBlockStatus.h"

#include "bc_status_cache/BlockchainStatusCache.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_base_utxo_index/UtxoData.h"

#include "bc_block_body/CoinbaseTransaction.h"
#include "bc_block_body/BlockRewordBase.h"
#include "bc_block_body/StakeBaseTransaction.h"

#include "bc_block/BlockHeaderId.h"

#include "transaction/AbstractSmartcontractTransaction.h"

#include "bc_status_cache_lockin/ILockinManager.h"
#include "bc_status_cache/ZoneStatusCache.h"

#include "bc_status_cache_data/FinalizedDataCache.h"

#include "bc_network/NodeIdentifier.h"

#include "bc_status_cache_lockin/LockInOperationsData.h"

#include "numeric/BigInteger.h"

#include "merkletree/MerkleCertificate.h"

#include "bc_block_header_command/AbstractBlockHeaderCommand.h"


namespace codablecash {

uint64_t StatusCacheContext::getSerial() noexcept {
	static uint64_t serial = 0;
	return ++serial;
}


StatusCacheContext::StatusCacheContext(const CodablecashSystemParam* config, const File* tmpCacheBaseDir, uint16_t zone
		, ConcurrentGate* rwLock, BlockchainStatusCache* statusCache, CodablecashBlockchain* blockchain, const wchar_t* prefix) {
	this->config = config;
	this->zone = zone;
	this->rwLock = rwLock;

	this->ticketPrice = 0;

	this->statusCache = statusCache;
	this->blockchain = blockchain;

	{
		char serialch[4];
		Mem::memset(serialch, 0, 4);
		uint64_t serial = getSerial();
		::sprintf(serialch, "%03ld", serial);

		UnicodeString baseStr(prefix);
		UnicodeString postFix(serialch);

		baseStr.append(&postFix);

		this->baseDir = tmpCacheBaseDir->get(&baseStr);
		this->baseDir->deleteDir();
		this->baseDir->mkdirs();
	}

	this->trxCache = nullptr;
	this->utxoCache = nullptr;
	this->voterCache = nullptr;

	this->topHeight = 0;

	this->numZones = statusCache->getNumZones();
	this->requestedNewShards = statusCache->getRequestedNewShards(zone);

	RemoteUtxoRepository* remoteUtxoRepo = statusCache->getRemoteUtxoRepository(zone);
	uint64_t finalizedHeight = statusCache->getFinalizedHeight(zone);
	const BlockchainSoftwareVersion* version = blockchain->getVersion();
	this->remoteUtxos = new RemoteUtxoDetector(remoteUtxoRepo, finalizedHeight, config, version);
}

StatusCacheContext::~StatusCacheContext() {
	close();

	if(this->rwLock != nullptr){
		this->rwLock->open();
	}
	this->rwLock = nullptr;

	delete this->remoteUtxos;
}

void StatusCacheContext::close() {
	delete this->trxCache, this->trxCache = nullptr;
	delete this->utxoCache, this->utxoCache = nullptr;
	delete this->voterCache, this->voterCache = nullptr;

	if(this->baseDir != nullptr && this->baseDir->exists()){
		this->baseDir->deleteDir();
	}
	delete this->baseDir, this->baseDir = nullptr;
}

void StatusCacheContext::init() {
	this->trxCache = new TransactionContextCache(this->baseDir);
	this->trxCache->init();
	this->trxCache->open();

	this->utxoCache = new UtxoCacheContext(this->baseDir);
	this->utxoCache->init();
	this->utxoCache->open();

	this->voterCache = new VoterStatusMappedCacheContext(this->baseDir);
	this->voterCache->init();

	this->ticketPrice = this->statusCache->getFinalizedTicketPrice(zone);
}

void StatusCacheContext::importBlock(const BlockHeader *header, const BlockBody *blockBody, ISystemLogger* logger) {
	LockinManager* liManager = this->statusCache->getLockInManager(this->zone);
	NullLockinManager lockinManager(liManager);

	beginBlock(header, &lockinManager, false);

	importControlTransactions(header, blockBody, logger);
	importInterChainCommunicationTransactions(header, blockBody, logger);
	importBalanceTransactions(header, blockBody, logger);
	importSmartcontractTransactions(header, blockBody, logger);
	importRewordTransactions(header, blockBody, logger);

	endBlock(header, &lockinManager, false);
}

void StatusCacheContext::beginBlock(const BlockHeader *header, ILockinManager* lockinManager, bool finalize) {
	// [multishard] header commands
	if(finalize){
		handleHeaderCommands(header, lockinManager);
	}

	// select voters
	ArrayList<VoterEntry, VoterEntry::VoteCompare>* list = getVoterEntries(); __STP(list);
	list->setDeleteOnExit();

	{
		uint64_t height = header->getHeight();
		LockInOperationsData* lockins = lockinManager->getOperantions(height); __STP(lockins);
		if(lockins != nullptr){
			lockins->apply(header, this);
		}
	}

#ifdef __DEBUG__
	{
		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			VoterEntry* entry = list->get(i);

			bool test = entry->checkTickets();
			assert(test == true);
		}
	}
#endif

	uint64_t height = header->getHeight();
	TicketVoteSelector selector(list, height, this->config);
	selector.select();

	{
		const ArrayList<const VoteTicket>* selectedlist = selector.getSelectedList();
		VotingBlockStatus* status = VotingBlockStatus::toVotingBlockStatus(header, selectedlist); __STP(status);
		this->voterCache->storeVotingBlockStatus(status);

		if(!status->isEmpty()){
			this->voterCache->removeVotedTicket(status);
		}
	}
}

void StatusCacheContext::handleHeaderCommands(const BlockHeader *header, ILockinManager *lockinManager) {
	// [multishard] header commands
	ArrayList<AbstractBlockHeaderCommand>* list = header->getHeaderCommands();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBlockHeaderCommand* cmd = list->get(i);

		cmd->onFinalize(header, this->statusCache, this->blockchain, lockinManager, this->config);
	}
}

void StatusCacheContext::endBlock(const BlockHeader *header, ILockinManager* lockinManager, bool finalize) {
	// update voted status check voted
	// This is what lockin operation have to do.

	/**
	 * status of block height, which tickets of the block height to include in this block
	 */
	VotingBlockStatus* status = getVotingBlockStatus(header); __STP(status);
	if(finalize == true && status != nullptr){ // on Finalizing
		uint64_t height = header->getHeight();

		int missingLimit = this->config->getVoteMissingLimit(height);
		int extendCount = this->config->getVoteExtendCapacityCount(height);

		// generate lockin inside here
		this->voterCache->handleVotedStatus(height, status, missingLimit, extendCount, lockinManager, this->config);
	}

	// generate price lockin inside here
	calcTicketPrice(header);
}

void StatusCacheContext::calcTicketPrice(const BlockHeader *header) {
	uint64_t height = header->getHeight();

	uint64_t window = this->config->getTicketPriceWindow(height);

	uint64_t mod = (height + 1) % window;
	if(mod != 0){
		return;
	}

	uint64_t numTicketMax = this->config->getTicketIdealNumber(height);
	uint64_t ticketPriceDefault = this->config->getTicketPriceDefault(height);

	ArrayList<VoterEntry, VoterEntry::VoteCompare>* list = getVoterEntries(); __STP(list);
	list->setDeleteOnExit();

	BalanceUnit total(0L);

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		VoterEntry* entry = list->get(i);

		total += entry->getTicketPriceSum();
	}

	uint64_t price = total.getAmount() / numTicketMax;
	price = price < ticketPriceDefault ? ticketPriceDefault : price;

	this->ticketPrice = price;
}

void StatusCacheContext::importBalanceTransactions(const BlockHeader *header, const BlockBody *blockBody, ISystemLogger* logger) {
	const ArrayList<AbstractBalanceTransaction>* list = blockBody->getBalanceTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBalanceTransaction* trx = list->get(i);

		importBalanceTransaction(header, trx, logger);
	}
}

void StatusCacheContext::importInterChainCommunicationTransactions(const BlockHeader *header, const BlockBody *blockBody, ISystemLogger* logger) {
	ArrayList<AbstractInterChainCommunicationTansaction>* list = blockBody->getInterChainCommunicationTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractInterChainCommunicationTansaction* trx = list->get(i);

		importInterChainCommunicationTransaction(header, trx, logger);
	}
}

void StatusCacheContext::importSmartcontractTransactions(const BlockHeader *header, const BlockBody *blockBody, ISystemLogger* logger) {
	ArrayList<AbstractSmartcontractTransaction>* list = blockBody->getSmartcontractTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractSmartcontractTransaction* trx = list->get(i);

		importSmartcontractTransaction(header, trx, logger);
	}
}

void StatusCacheContext::importBalanceTransaction(const BlockHeader *header, const AbstractBalanceTransaction *trx, ISystemLogger* logger) {
	this->trxCache->putTransaction(trx);

	importUtxo(trx, header);
}

void StatusCacheContext::importControlTransactions(const BlockHeader *header, const BlockBody *blockBody, ISystemLogger* logger) {
	const ArrayList<AbstractControlTransaction>* list = blockBody->getControlTransactions();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractControlTransaction* trx = list->get(i);

		importControlTransaction(header, blockBody, trx, logger);
	}
}


void StatusCacheContext::importRewordTransactions(const BlockHeader *header,	const BlockBody *blockBody, ISystemLogger* logger) {
	const BlockRewordBase* rewardBase = blockBody->getBlockRewordBase();

	const CoinbaseTransaction* coinbaseTrx = rewardBase->getCoinbaseTransaction();
	if(coinbaseTrx != nullptr){
		importUtxo(coinbaseTrx, header);
	}

	const ArrayList<StakeBaseTransaction>* stakebases = rewardBase->getStakeBases();

	int maxLoop = stakebases->size();
	for(int i = 0; i != maxLoop; ++i){
		const StakeBaseTransaction* trx = stakebases->get(i);

		importUtxo(trx, header);
	}
}

void StatusCacheContext::importInterChainCommunicationTransaction(	const BlockHeader *header, const AbstractInterChainCommunicationTansaction *trx, ISystemLogger* logger) {
#ifdef __DEBUG__
	bool bl = this->trxCache->hasTransaction(trx->getTransactionId());
	assert(bl == false);
#endif

	this->trxCache->putTransaction(trx);

	importUtxo(trx, header);
}

void StatusCacheContext::importSmartcontractTransaction(const BlockHeader *header, const AbstractSmartcontractTransaction *trx, ISystemLogger* logger) {
#ifdef __DEBUG__
	bool bl = this->trxCache->hasTransaction(trx->getTransactionId());
	assert(bl == false);
#endif

	this->trxCache->putTransaction(trx);

	importUtxo(trx, header);
}

void StatusCacheContext::importControlTransaction(const BlockHeader *header,  const BlockBody* body, const AbstractControlTransaction *trx, ISystemLogger* logger) {
#ifdef __DEBUG__
	bool bl = this->trxCache->hasTransaction(trx->getTransactionId());
	assert(bl == false);
#endif

	this->trxCache->putTransaction(trx);

	// handle utxo
	importUtxo(trx, header);

	uint8_t type = trx->getType();
	if(type == AbstractBlockchainTransaction::TRX_TYPE_REGISTER_VOTE_POOL){
		const RegisterVotePoolTransaction* registerTrx = dynamic_cast<const RegisterVotePoolTransaction*>(trx);
		registerVoterPool(registerTrx, header->getHeight(), header, body);
	}
	else if(type == AbstractBlockchainTransaction::TRX_TYPE_REGISTER_TICKET){
		const RegisterTicketTransaction* ticketTrx = dynamic_cast<const RegisterTicketTransaction*>(trx);
		registerTicket(header, ticketTrx);
	}
	else if(type == AbstractBlockchainTransaction::TRX_TYPE_VOTE_BLOCK){
		const VoteBlockTransaction* voteTrx = dynamic_cast<const VoteBlockTransaction*>(trx);
		registerVote(header, voteTrx);
	}
	else if(type == AbstractBlockchainTransaction::TRX_TYPE_REVOKE_MISSED_TICKET){
		// do nothing about controlling
	}
	else if(type == AbstractBlockchainTransaction::TRX_TYPE_REVOKE_MISS_VOTED_TICKET){
		// do nothing about controlling
	}
}

void StatusCacheContext::importUtxo(const AbstractBlockchainTransaction *trx, const BlockHeader *header) {
	{
		int maxLoop = trx->getUtxoSize();
		for(int i = 0; i != maxLoop; ++i){
			AbstractUtxo* utxo = trx->getUtxo(i);

			putUtxo(utxo, trx, header);
		}
	}

	{
		int maxLoop = trx->getUtxoReferenceSize();
		for(int i = 0; i != maxLoop; ++i){
			AbstractUtxoReference* ref = trx->getUtxoReference(i);

			// coinbase && stakebase
			uint8_t type = ref->getType();
			if(type == AbstractUtxoReference::UTXO_REF_TYPE_COINBASE ||
					type == AbstractUtxoReference::UTXO_REF_TYPE_STAKEBASE ||
					ref->isRemote()){
				continue;
			}

			removeUtxo(ref, trx);
		}
	}
}

AbstractUtxo* StatusCacheContext::getUtxo(const UtxoId *utxoId) const {
	// active
	{
		UtxoData* data = this->utxoCache->getData(utxoId); __STP(data);
		if(data != nullptr && data->isRemoved()){ // explicitly removes
			return nullptr;
		}

		if(data != nullptr){
			const AbstractUtxo* utxo = data->getUtxo();
			return dynamic_cast<AbstractUtxo*>(utxo->copyData());
		}
	}

	// finalized data
	UtxoData* data = this->statusCache->findUtxo(zone, utxoId); __STP(data);
	return data != nullptr ? dynamic_cast<AbstractUtxo*>(data->getUtxo()->copyData()) : nullptr;
}

void StatusCacheContext::putUtxo(const AbstractUtxo *utxo, const AbstractBlockchainTransaction *trx, const BlockHeader *header) {
#ifdef __DEBUG__
	// check cache is not removed
	{
		const UtxoId* id = utxo->getId();
		AbstractUtxo* __u = getUtxo(id); __STP(__u);
		assert(__u == nullptr);
	}
#endif

	uint64_t height = header->getHeight();
	this->utxoCache->putUtxo(utxo, trx->getTransactionId(), height);
}

void StatusCacheContext::removeUtxo(const AbstractUtxoReference *utxoref, const AbstractBlockchainTransaction *trx) {
#ifdef __DEBUG__
	// check cache is not removed
	{
		const UtxoId* id = utxoref->getUtxoId();
		AbstractUtxo* __u = getUtxo(id); __STP(__u);
		assert(__u != nullptr);
	}
#endif

	const UtxoId* utxoId = utxoref->getUtxoId();
	this->utxoCache->removeUtxo(utxoId, trx);
}

ArrayList<VoterEntry, VoterEntry::VoteCompare>* StatusCacheContext::getVoterEntries() const {
	ArrayList<VoterEntry, VoterEntry::VoteCompare>* list = new ArrayList<VoterEntry, VoterEntry::VoteCompare>();

	this->voterCache->listEntries(list);

	return list;
}

void StatusCacheContext::registerVoterPool(const RegisterVotePoolTransaction *trx, uint64_t blockHeight, const BlockHeader *header, const BlockBody* body) {
	const NodeIdentifier* nodeId = trx->getNodeId();

	// voter address
	const AddressDescriptor* voterAddress = trx->getAddressDesc();

	VoterEntry* entry = VoterEntry::createVoteEntry(nodeId, voterAddress);
	this->voterCache->addNewVoter(entry);

	int cap = this->config->getVoteDefaultCapacity(blockHeight);
	entry->setCapacity(cap);


	const BlockMerkleRoot* root = body->getMerkleRoot();
	if(root != nullptr){
		const BigInteger* pub = nodeId->getPublicKey();
		ByteBuffer* buff = pub->toBinary(); __STP(buff);

		MerkleCertificate* merkleCert = body->makeCertificate((const char*)buff->array(), buff->capacity()); __STP(merkleCert);
		entry->setNodeIdMerkleCert(merkleCert);
	}
}

void StatusCacheContext::registerTicket(const BlockHeader *header, const RegisterTicketTransaction *trx) {
	TicketUtxo* ticketUtxo = trx->getTicketUtxo();
	const NodeIdentifier* nodeId = ticketUtxo->getNodeIdentifier();

	const VoterEntry* entry = getVoterEntry(nodeId);
	assert(entry != nullptr);

	//　add ticket
#ifdef __DEBUG__
{
	bool test = entry->checkTickets();
	assert(test == true);

	const UtxoId* utxoId = ticketUtxo->getId();
	const VoteTicket* last = entry->getTicket(utxoId);
	assert(last == nullptr);
}
#endif

	const AddressDescriptor* voterAddress = entry->getVoterAddressDescriptor();

	VoteTicket* ticket = VoteTicket::toVoteTicket(header->getHeight(), ticketUtxo, voterAddress); __STP(ticket);

	const_cast<VoterEntry*>(entry)->addTicket(ticket);

#ifdef __DEBUG__
	{
		bool test = entry->checkTickets();
		assert(test == true);

		const VoteTicket* last = entry->getTicket(ticketUtxo->getId());
		assert(last != nullptr);
	}
#endif
}

void StatusCacheContext::registerVote(const BlockHeader *header, const VoteBlockTransaction *trx) {
	VotingBlockStatus* status = getVotingBlockStatus(header); __STP(status);
	if(status != nullptr){
		// get Correct Header
		BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(this->zone);

		uint64_t height = header->getHeight();
		const BlockHeaderId* lastHeaderId = header->getLastHeaderId();

		uint16_t beforeN = this->config->getVoteBeforeNBlocks(height);
		uint16_t afterN = this->config->getVoteBlockIncludeAfterNBlocks(height);

		uint64_t voteBeforeNBlocks = beforeN + afterN - 1;
		BlockHeader* votedHeader = headerManager->getNBlocksBefore(lastHeaderId, height - 1, voteBeforeNBlocks); __STP(votedHeader); // correct header

		const BlockHeaderId* correctHeaderId = votedHeader->getId(); // get Correct Header Id

		if(correctHeaderId->equals(trx->getVotedHeaderId())){
			const TicketUtxoReference* ref = trx->getTicketUtxoReference();
			const UtxoId* utxoId = ref->getUtxoId();

			this->voterCache->handleVote(status, utxoId);

			// write back status
			this->voterCache->storeVotingBlockStatus(status);
		}
	}
}

const VoterEntry* StatusCacheContext::getVoterEntry(const NodeIdentifier *nodeId) const noexcept {
	const VoterEntry* entry = this->voterCache->getVoterEntry(nodeId);

	return entry;
}

void StatusCacheContext::loadInitialVotersData() {
	this->voterCache->loadFinalyzedVoters(this->zone, this->statusCache);

	{
		ZoneStatusCache* cache = this->statusCache->getZoneStatusCache(this->zone);
		FinalizedDataCache* fcache = cache->getFinalizedDataCache();
		VoterStatusCacheContext* statusCacheContext = fcache->getVotingStatusCache();

		this->voterCache->importRepo(statusCacheContext);
	}
}

VotingBlockStatus* StatusCacheContext::getVotingBlockStatus(const BlockHeaderId *blockHeaderId) {
	VotingBlockStatus* status = this->voterCache->getVotingBlockStatus(blockHeaderId);

	return status;
}

VotingBlockStatus* StatusCacheContext::getVotingBlockStatus(const BlockHeader *header) {
	uint64_t height = header->getHeight();
	const BlockHeaderId* headerId = header->getId();

	// get previous block to vote
	uint16_t voteBlockIncludeAfterNBlocks = this->config->getVoteBlockIncludeAfterNBlocks(height);
	if(height <= voteBlockIncludeAfterNBlocks){
		return nullptr;
	}

	BlockHeaderStoreManager* headerManager = this->blockchain->getHeaderManager(this->zone);

	// check if header is latest which is not mined
	{
		BlockHeader* thisHeader = headerManager->getHeader(headerId, height); __STP(thisHeader);
		if(thisHeader == nullptr){
			headerId = header->getLastHeaderId();
			--height;
			--voteBlockIncludeAfterNBlocks;
		}
	}

	BlockHeader* votedHeightBlockHeader = headerManager->getNBlocksBefore(headerId, height, voteBlockIncludeAfterNBlocks); __STP(votedHeightBlockHeader);
	const BlockHeaderId* votedHeightHeaderId = votedHeightBlockHeader->getId();

	// check voted
	VotingBlockStatus* status = getVotingBlockStatus(votedHeightHeaderId); __STP(status); // virtual function

	return __STP_MV(status);
}

uint64_t StatusCacheContext::getPreAnalyzedHeight() const noexcept {
	return this->statusCache->getFinalizedHeight(this->zone);
}

AbstractBlockchainTransaction* StatusCacheContext::getTransaction(const TransactionId *trxId) {
	return this->trxCache->getTransaction(trxId);
}

void StatusCacheContext::setNumZones(uint16_t numZones) noexcept {
	this->numZones = numZones;
}

void StatusCacheContext::setRequestedNewShards(int requestedNewShards) noexcept {
	this->requestedNewShards = requestedNewShards;
}

} /* namespace codablecash */
