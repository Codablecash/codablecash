/*
 * CachedStatusCache.cpp
 *
 *  Created on: 2023/04/26
 *      Author: iizuka
 */

#include "bc_status_cache_context/CachedStatusCache.h"
#include "bc_status_cache_context/TransactionContextCache.h"
#include "bc_status_cache_context/UtxoCacheContext.h"
#include "bc_status_cache_context/StatusCacheContext.h"
#include "bc_status_cache_context/RemoteUtxoDetector.h"
#include "bc_status_cache_context/RemoteUtxoHeight.h"

#include "bc_status_cache_context_finalizer/VoterStatusMappedCacheContext.h"

#include <cstdio>

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_io/File.h"

#include "btree/BtreeScanner.h"

#include "filestore_block/IBlockObject.h"

#include "bc_base_trx_index/TransactionIdKey.h"
#include "bc_base_trx_index/TransactionData.h"
#include "bc_base_trx_index/TransactionIdData.h"

#include "bc_base_utxo_index/UtxoIdKey.h"
#include "bc_base_utxo_index/UtxoData.h"

#include "bc_trx/UtxoId.h"


namespace codablecash {

CachedStatusCache::CachedStatusCache(uint64_t serial, const File* cacheRepoBaseDir, uint64_t height) {
	this->height = height;

	char serialch[4];
	Mem::memset(serialch, 0, 4);
	::sprintf(serialch, "%03ld", serial);
	UnicodeString baseStr(CACHED_STATUS_PREFIX);
	UnicodeString postFix(serialch);
	baseStr.append(&postFix);

	this->baseDir = cacheRepoBaseDir->get(&baseStr);
	this->baseDir->deleteDir();
	this->baseDir->mkdirs();

	this->trxCache = nullptr;
	this->utxoCache = nullptr;
	this->voterCache = nullptr;

	this->ticketPrice = 0;

	// new shards history
	this->requestedNewShards = 0;
	this->numZones = 0;

	this->utxolist = new ArrayList<RemoteUtxoHeight>();
}

CachedStatusCache::~CachedStatusCache() {
	delete this->trxCache;
	delete this->utxoCache;
	delete this->voterCache;

	this->baseDir->deleteDir();
	delete this->baseDir;

	this->utxolist->deleteElements();
	delete this->utxolist;
}

void CachedStatusCache::init() {
	this->trxCache = new TransactionContextCache(this->baseDir);
	this->trxCache->init();
	this->trxCache->open();

	this->utxoCache = new UtxoCacheContext(this->baseDir);
	this->utxoCache->init();
	this->utxoCache->open();

	this->voterCache = new VoterStatusMappedCacheContext(this->baseDir);
	this->voterCache->init();
}

void CachedStatusCache::importStatusContext(StatusCacheContext *context) {
	// cache
	const CachedStatusCache* previousCache = context->getCachedStatusCache();
	if(previousCache != nullptr){
		importOtherCache(previousCache);
	}

	// StatusCacheContext
	importTransactions(context->getTransactionContextCache());
	importUxtoCache(context->getUtxoCacheContext());
	importVoterStatusCache(context->getVoterStatusCacheContext());

	this->ticketPrice = context->getTicketPrice();

	this->requestedNewShards = context->getRequestedNewShards();
	this->numZones = context->getNumZones();

	// remote utxos
	{
		RemoteUtxoDetector* remoteUtxos = context->getRemoteUtxoDetector();
		ArrayList<RemoteUtxoHeight>* list = remoteUtxos->getList();

		int maxLoop = list->size();
		for(int i = 0; i != maxLoop; ++i){
			const RemoteUtxoHeight* utxoIdHeight = list->get(i);

			this->utxolist->addElement(new RemoteUtxoHeight(*utxoIdHeight));
		}
	}
}

void CachedStatusCache::importOtherCache(const CachedStatusCache *previousCache) {
	importTransactions(previousCache->trxCache);
	importUxtoCache(previousCache->utxoCache);

	//importVoterStatusCache(previousCache->voterCache);
}

void CachedStatusCache::importTransactions(TransactionContextCache *cache) {
	BtreeScanner* scanner = cache->getBtreeScanner(); __STP(scanner);
	scanner->begin();

	while(scanner->hasNext()){
		const IBlockObject* obj = scanner->next();

		const TransactionData* data = dynamic_cast<const TransactionData*>(obj);

		const AbstractBlockchainTransaction* trx = data->getTrx();
		this->trxCache->putTransaction(trx);
	}
}

void CachedStatusCache::importUxtoCache(UtxoCacheContext *cache) {
	BtreeScanner* scanner = cache->getBtreeScanner(); __STP(scanner);
	scanner->begin();

	while(scanner->hasNext()){
		const IBlockObject* obj = scanner->next();

		const UtxoData* data = dynamic_cast<const UtxoData*>(obj);

		this->utxoCache->putUtxo(data);
	}
}

void CachedStatusCache::importVoterStatusCache(VoterStatusMappedCacheContext *cache) {
	this->voterCache->importMap(cache);
	this->voterCache->importRepo(cache);
}

UtxoData* CachedStatusCache::getData(const UtxoId *utxoId) const {
	return this->utxoCache->getData(utxoId);
}

AbstractBlockchainTransaction* CachedStatusCache::getTransaction(const TransactionId *trxId) const {
	return this->trxCache->getTransaction(trxId);
}

} /* namespace codablecash */
