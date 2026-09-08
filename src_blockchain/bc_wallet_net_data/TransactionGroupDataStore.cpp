/*
 * UnfinalizedChainDataStore.cpp
 *
 *  Created on: Apr 19, 2025
 *      Author: iizuka
 */

#include "bc_wallet_net_data/HeaderTransactionGroupDataFactory.h"
#include "bc_wallet_net_data/HeaderTransactionGroup.h"

#include "base_io/File.h"

#include "random_access_file/DiskCacheManager.h"

#include "bc_blockstore_header/BlockHeaderIdKeyFactory.h"
#include "bc_blockstore_header/BlockHeaderIdKey.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"
#include "bc_wallet_net_data/TransactionGroupDataStore.h"

#include "btree/Btree.h"
#include "btree/BtreeConfig.h"


namespace codablecash {

TransactionGroupDataStore::TransactionGroupDataStore(const File* baseDir) {
	this->baseDir = new File(*baseDir);
	this->cacheManager = new DiskCacheManager();
	this->headerGroupStore = nullptr;

}

TransactionGroupDataStore::~TransactionGroupDataStore() {
	close();

	delete this->baseDir;
	delete this->cacheManager;
}

void TransactionGroupDataStore::initBlank() {
	UnicodeString fileName(TransactionGroupDataStore::FILE_NAME);

	BlockHeaderIdKeyFactory* keyFactory = new BlockHeaderIdKeyFactory(); __STP(keyFactory);
	HeaderTransactionGroupDataFactory* dataFactory = new HeaderTransactionGroupDataFactory(); __STP(dataFactory);

	Btree btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);
	BtreeConfig config;
	config.nodeNumber = 8;
	config.defaultSize = 1024;
	config.blockSize = 32;

	btree.create(&config);
}

void TransactionGroupDataStore::open() {
	UnicodeString fileName(TransactionGroupDataStore::FILE_NAME);

	BlockHeaderIdKeyFactory* keyFactory = new BlockHeaderIdKeyFactory(); __STP(keyFactory);
	HeaderTransactionGroupDataFactory* dataFactory = new HeaderTransactionGroupDataFactory(); __STP(dataFactory);

	this->headerGroupStore = new Btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

	BtreeOpenConfig opconf;
	opconf.numDataBuffer = 256;
	opconf.numNodeBuffer = 512;

	this->headerGroupStore->open(&opconf);
}

void TransactionGroupDataStore::close() noexcept {
	if(this->headerGroupStore != nullptr){
		this->headerGroupStore->close();
		delete this->headerGroupStore, this->headerGroupStore = nullptr;
	}
}

void TransactionGroupDataStore::add(const BlockHeaderId *headerId, const HeaderTransactionGroup *group) {
	BlockHeaderIdKey key(headerId);

	this->headerGroupStore->putData(&key, group);
}

HeaderTransactionGroup* TransactionGroupDataStore::getHeaderTransactionGroup(const BlockHeaderId *headerId) const {
	BlockHeaderIdKey key(headerId);

	IBlockObject* object = this->headerGroupStore->findByKey(&key);
	HeaderTransactionGroup* trxGroup = dynamic_cast<HeaderTransactionGroup*>(object);

	assert(trxGroup != nullptr);

	return trxGroup;
}

bool TransactionGroupDataStore::removeHeaderTransactionGroup(const BlockHeaderId *headerId) {
	BlockHeaderIdKey key(headerId);

	bool ret = this->headerGroupStore->remove(&key);
	return ret;
}

} /* namespace codablecash */
