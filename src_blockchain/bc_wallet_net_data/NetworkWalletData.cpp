/*
 * NetworkCommandData.cpp
 *
 *  Created on: Mar 23, 2025
 *      Author: iizuka
 */

#include "bc_wallet_net_data/NetworkWalletData.h"
#include "bc_wallet_net_data/WalletMemoryPool.h"
#include "bc_wallet_net_data/HeaderTransactionGroup.h"
#include "bc_wallet_net_data/TransactionGroupDataStore.h"

#include "bc_wallet_net_data_management/ManagementAccountsCollection.h"
#include "bc_wallet_net_data_management/ManagementAccount.h"

#include "bc_wallet/HdWallet.h"
#include "bc_wallet/WalletAccount.h"

#include "base/StackRelease.h"

#include "base_io/File.h"

#include "bc_blockstore_header/BlockHeaderStoreManager.h"

#include "bc_blockstore/CodablecashBlockchain.h"

#include "bc_status_cache/HeadBlockDetector.h"
#include "bc_status_cache/BlockHeadElement.h"

#include "bc_block/BlockHeader.h"
#include "bc_block/BlockHeaderId.h"

#include "bc/CodablecashSystemParam.h"
#include "bc/ISystemLogger.h"

#include "bc_base_conf_store/StatusStore.h"

#include "bc_trx/AbstractBlockchainTransaction.h"

#include "btree/BtreeScanner.h"

#include "bc_base_trx_index/TransactionData.h"

#include "base_thread/ConcurrentGate.h"

#include "data_history_data/TransactionTransferData.h"


using alinous::StackWriteLock;

namespace codablecash {

NetworkWalletData::NetworkWalletData(const File* baseDir, ISystemLogger* logger, const CodablecashSystemParam *config) {
	this->baseDir = new File(*baseDir);
	this->logger = logger;
	this->config = config;

	this->hdWallet = nullptr;
	this->headerManager = nullptr;
	this->detector = nullptr;

	this->transactionGroupData = new TransactionGroupDataStore(this->baseDir);

	File* mempoolBaseDir = this->baseDir->get(L"mempool"); __STP(mempoolBaseDir);
	this->mempool = new WalletMemoryPool(mempoolBaseDir);

	this->finalizedHeight = 0;
	this->zone = 0;
	this->statusStore = nullptr;

	this->managementAccounts = new ManagementAccountsCollection();

	this->gateLock = new ConcurrentGate();
}

NetworkWalletData::~NetworkWalletData() {
	close();

	delete this->hdWallet;

	delete this->mempool;

	delete this->headerManager;
	delete this->detector;

	delete this->baseDir;
	this->logger = nullptr;

	delete this->managementAccounts;

	delete this->gateLock;
}

void NetworkWalletData::close() noexcept {
	StackWriteLock __lock(this->gateLock, __FILE__, __LINE__);

	if(this->transactionGroupData != nullptr){
		this->transactionGroupData->close();
		delete this->transactionGroupData;
		this->transactionGroupData = nullptr;
	}

	if(this->mempool != nullptr){
		this->mempool->close();
		delete this->mempool;
		this->mempool = nullptr;
	}

	if(this->statusStore != nullptr){
		this->statusStore->close();
		delete this->statusStore;
		this->statusStore = nullptr;
	}
}

void NetworkWalletData::createHdWallet(const HdWalletSeed *seed, uint16_t defaultZone, const IWalletDataEncoder *encoder, int defaultMaxAddress) {
	StackWriteLock __lock(this->gateLock, __FILE__, __LINE__);

	delete this->hdWallet;

	File* hdbase = this->baseDir->get(L"hd"); __STP(hdbase);

	this->hdWallet = HdWallet::create(hdbase, seed, defaultZone, encoder, defaultMaxAddress);

	this->zone = defaultZone;

	__initStatusStore();
	__saveStatus();
}

void NetworkWalletData::createBlank() {
	StackWriteLock __lock(this->gateLock, __FILE__, __LINE__);

	this->headerManager = new BlockHeaderStoreManager(this->baseDir, CodablecashBlockchain::DEFAULT_SECTION_LIMIT);
	this->detector = new HeadBlockDetector(this->logger);

	this->transactionGroupData->initBlank();
	this->transactionGroupData->open();

	this->mempool->createBlankPool();
	this->mempool->open();

	__initStatusStore();
	__saveStatus();
}

void NetworkWalletData::addTransactionDataToMempool(const TransactionTransferData *data) {
	StackWriteLock __lock(this->gateLock, __FILE__, __LINE__);

	AbstractBlockchainTransaction* trx = data->getTransaction();
	const TransactionId* trxId = trx->getTransactionId();

	if(!this->mempool->hasTransaction(trxId)){
		this->mempool->putTransaction(trx);

		this->managementAccounts->resetMempool();
		__buildMempoolAccount();
	}
}

uint8_t NetworkWalletData::getTransactionStoreStatus(const TransactionId *trxId) const noexcept {
	StackReadLock __lock(this->gateLock, __FILE__, __LINE__);

	return this->managementAccounts->getTransactionStoreStatus(trxId);
}

uint16_t NetworkWalletData::getDefaultZone() const noexcept {
	StackReadLock __lock(this->gateLock, __FILE__, __LINE__);

	return this->hdWallet->getDefaultZone();
}

void NetworkWalletData::addHeader(const BlockHeader *header, const ArrayList<AbstractBlockchainTransaction>* trxlist) {
	StackWriteLock __lock(this->gateLock, __FILE__, __LINE__);

	// add header if has header
	{
		const BlockHeaderId* headerId = header->getId();
		uint64_t height = header->getHeight();

		BlockHeader* h = headerManager->getHeader(headerId, height); __STP(h);
		if(h == nullptr){
			this->headerManager->addHeader(header);
		}
	}

	// trx group
	{
		HeaderTransactionGroup group;
		const BlockHeaderId* id = header->getId();
		group.setHeaderId(id);

		int maxLoop = trxlist->size();
		for(int i = 0; i != maxLoop; ++i){
			const AbstractBlockchainTransaction* trx = trxlist->get(i);

			group.addTransaction(trx);
		}

		this->transactionGroupData->add(id, &group);
	}
}

bool NetworkWalletData::checkAndFinalizing() {
	StackWriteLock __lock(this->gateLock, __FILE__, __LINE__);

	bool finalized = false;

	int votePerBlock = this->config->getVotePerBlock();
	ArrayList<const BlockHeader> list;

	// make list
	{
		const BlockHead* head = this->detector->getHead();
		const ArrayList<BlockHeadElement>* elements = head->getHeaders();

		int maxLoop = elements->size();
		for(int i = 0; i != maxLoop; ++i){
			const BlockHeadElement* ele = elements->get(i);
			const BlockHeader* header = ele->getBlockHeader();
			uint64_t height = header->getHeight();

			if(height > this->finalizedHeight){
				list.addElement(header);
			}
		}
	}

	// handle the list
	{
		int maxLoop = list.size();
		for(int i = 0; i != maxLoop; ++i){
			const BlockHeader* header = list.get(i);

			if(header->isFinalizing(votePerBlock)){
				__doFinalize(header);
				finalized = true;
			}
		}
	}

	return finalized;
}

void NetworkWalletData::__doFinalize(const BlockHeader *header) {
	uint64_t includingHeight = header->getHeight();
	const BlockHeaderId* id = header->getId();

	uint16_t voteBeforeNBlocks = this->config->getVoteBeforeNBlocks(includingHeight);
	uint16_t voteBlockIncludeAfterNBlocks = this->config->getVoteBlockIncludeAfterNBlocks(includingHeight);
	int beforeHeight = voteBeforeNBlocks + voteBlockIncludeAfterNBlocks;

	uint64_t finalizingHeight = includingHeight - beforeHeight;

	if(this->finalizedHeight < finalizingHeight){
		BlockHeader* finalizedheader = this->headerManager->getNBlocksBefore(id, includingHeight, beforeHeight); __STP(finalizedheader);

#ifdef __DEBUG__
		assert(finalizedheader != nullptr);
		assert(finalizingHeight == finalizedheader->getHeight());

		{
			UnicodeString* strId = id->toString(); __STP(strId);

			UnicodeString message(L"  [Wallet Finalize Header] Height: ");
			message.append((int64_t)finalizingHeight);
			message.append(L" Header Id: ").append(strId);

			this->logger->log(&message);
		}
#endif

		const BlockHeaderId *fheaderId = finalizedheader->getId();
		__updateFinalizedData(finalizingHeight, fheaderId);

		// update
		this->finalizedHeight = finalizingHeight;
		__saveStatus();
	}
}

void NetworkWalletData::__updateFinalizedData(uint64_t finalizingHeight, const BlockHeaderId *finalizingHeaderId) {
	uint64_t lastFinalizedHeight = this->finalizedHeight;

	ArrayList<BlockHeader> list;
	list.setDeleteOnExit();

	// make list
	{
		uint64_t height = finalizingHeight;
		BlockHeaderId* currentHeaderId = dynamic_cast<BlockHeaderId*>(finalizingHeaderId->copyData());

		while(height > lastFinalizedHeight){
			__STP(currentHeaderId);

			BlockHeader* header = this->headerManager->getHeader(currentHeaderId, height); __STP(header);
			assert(header != nullptr);

			list.addElement(dynamic_cast<BlockHeader*>(header->copyData()), 0);

			// last header
			currentHeaderId = dynamic_cast<BlockHeaderId*>(header->getLastHeaderId()->copyData());
			height--;
		}
		__STP(currentHeaderId);
	}

	// to Hd wallet
	__importIntoHdWallet(&list);


	// clean header store
	__finalizeHeaderStore(finalizingHeight, finalizingHeaderId);

}

void NetworkWalletData::__importIntoHdWallet(const ArrayList<BlockHeader> *list) {
	WalletAccount* waccount = this->hdWallet->getZoneAccount(this->zone);

	// import
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const BlockHeader* header = list->get(i);
		const BlockHeaderId* headerId = header->getId();

		HeaderTransactionGroup* trxGourp = this->transactionGroupData->getHeaderTransactionGroup(headerId); __STP(trxGourp);
		__importImportHeaderTransactionGroup(trxGourp, waccount);
	}

	// clean
	for(int i = 0; i != maxLoop; ++i){
		const BlockHeader* header = list->get(i);
		const BlockHeaderId* headerId = header->getId();

		this->transactionGroupData->removeHeaderTransactionGroup(headerId);
	}
}

void NetworkWalletData::__importImportHeaderTransactionGroup(const HeaderTransactionGroup *trxGourp, WalletAccount *waccount) {
	const ArrayList<AbstractBlockchainTransaction>* list = trxGourp->getTransactionsList();

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const AbstractBlockchainTransaction* trx = list->get(i);

		waccount->importTransaction(trx);
	}
}

void NetworkWalletData::__finalizeHeaderStore(uint64_t height, const BlockHeaderId *headerId) {
	this->headerManager->finalize(height, headerId, nullptr);
}


void NetworkWalletData::updateHeadDetection() {
	StackWriteLock __lock(this->gateLock, __FILE__, __LINE__);

	// detect header
	this->detector->reset();
	// Make header lines
	this->detector->buildHeads(this->zone, this, this->finalizedHeight);

	// evaluate
	this->detector->evaluate(this->zone, nullptr, this, this->config, true);
	this->detector->selectChain();
}

void NetworkWalletData::__saveStatus() {
	this->statusStore->addShortValue(KEY_ZONE, this->zone);
	this->statusStore->addLongValue(KEY_FINALIZED_HEIGHT, this->finalizedHeight);
}

BlockHeaderStoreManager* NetworkWalletData::getHeaderManager(uint16_t zone) const noexcept {
	return this->headerManager;
}

BlockBodyStoreManager* NetworkWalletData::getBlockBodyStoreManager(uint16_t zone) const noexcept {
	return nullptr;
}

void NetworkWalletData::__initStatusStore() {
	if(this->statusStore == nullptr){
		this->statusStore = new StatusStore(this->baseDir, STATUS_STORE_FILE_NAME);
	}
}

void NetworkWalletData::resetManagementAccounts() noexcept {
	StackWriteLock __lock(this->gateLock, __FILE__, __LINE__);

	this->managementAccounts->resetAll();
}

void NetworkWalletData::buildManagementAccount(bool buildFinalized, uint64_t startHeight) {
	StackWriteLock __lock(this->gateLock, __FILE__, __LINE__);

	if(buildFinalized){
		__buildFinalizedManagementAccount(startHeight);
	}

	// unfinalized
	__buildUnfinalizedAccount();

	// mempool
	__buildMempoolAccount();
}

void NetworkWalletData::__buildMempoolAccount() {
	WalletAccount* waccount = this->hdWallet->getZoneAccount(this->zone);

	ManagementAccount* uma = this->managementAccounts->getUnFinalizedManagementAccount();
	ManagementAccount* ma = this->managementAccounts->getMempoolManagementAccount();

	ma->reset();
	ma->importOtherAccount(uma);

	{
		BtreeScanner* scanner = this->mempool->getScanner(); __STP(scanner);
		scanner->begin();

		while(scanner->hasNext()){
			const IBlockObject* obj = scanner->next();
			const TransactionData* data = dynamic_cast<const TransactionData*>(obj);

			const AbstractBlockchainTransaction* trx = data->getTrx();
			ma->addTransaction(trx, waccount);
		}
	}
}

void NetworkWalletData::__buildUnfinalizedAccount() {
	ManagementAccount* fma = this->managementAccounts->getFinalizedManagementAccount();
	ManagementAccount* ma = this->managementAccounts->getUnFinalizedManagementAccount();

	ma->reset();
	ma->importOtherAccount(fma);

	// from detector
	{
		const BlockHead* head = this->detector->getHead();
		const ArrayList<BlockHeadElement>* list = head->getHeaders();

		int maxLoop = list->size();

		// i = 1, because the first element is finalized height
		for(int i = 1; i != maxLoop; ++i){
			const BlockHeadElement* element = list->get(i);
			const BlockHeader* header = element->getBlockHeader();

			__buildManagementAccount4Header(ma, header);
		}
	}
}

void NetworkWalletData::__buildFinalizedManagementAccount(uint64_t startHeight) {
	ManagementAccount *ma = this->managementAccounts->getFinalizedManagementAccount();
	uint64_t maxBlockHeight = this->finalizedHeight;

	__buildFinalizedManagementAccount(ma, startHeight, maxBlockHeight);
}

void NetworkWalletData::__buildFinalizedManagementAccount(ManagementAccount *ma, uint64_t startHeight, uint64_t maxBlockHeight) {
	WalletAccount* waccount = this->hdWallet->getZoneAccount(this->zone);

	ArrayList<AbstractBlockchainTransaction>* list = waccount->getTransactions(); __STP(list);
	list->setDeleteOnExit();

	// import Hd wallet into Management Account
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const AbstractBlockchainTransaction* trx = list->get(i);

		if(trx->checkFilteredUxtoRef(ma) || trx->checkFilteredAddress(waccount)){
			ma->addTransaction(trx, waccount);
		}
	}
}

void NetworkWalletData::__buildManagementAccount4Header(ManagementAccount *ma, const BlockHeader *header) {
	WalletAccount* waccount = this->hdWallet->getZoneAccount(this->zone);

	uint64_t height = header->getHeight();
	const BlockHeaderId* headerId = header->getId();

	HeaderTransactionGroup* trxGroup = this->transactionGroupData->getHeaderTransactionGroup(headerId); __STP(trxGroup);

	const ArrayList<AbstractBlockchainTransaction>* list = trxGroup->getTransactionsList();
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const AbstractBlockchainTransaction* trx = list->get(i);

		// check trx contains address
		if(trx->checkFilteredUxtoRef(ma) || trx->checkFilteredAddress(waccount)){
			ma->addTransaction(trx, waccount);
		}
	}
}

uint64_t NetworkWalletData::getFinalizedHeight() const {
	StackReadLock __lock(this->gateLock, __FILE__, __LINE__);

	return this->finalizedHeight;
}

} /* namespace codablecash */
