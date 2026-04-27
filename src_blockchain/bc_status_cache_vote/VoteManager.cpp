/*
 * VoteManager.cpp
 *
 *  Created on: 2024/04/29
 *      Author: iizuka
 */

#include "bc_status_cache_vote/VoteManager.h"
#include "bc_status_cache_vote/HeightVoteDataFactory.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "base_io/File.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"

#include "random_access_file/DiskCacheManager.h"

#include "btree/Btree.h"
#include "btree/BtreeConfig.h"

#include "btreekey/BtreeKeyFactory.h"
#include "btreekey/ULongKey.h"

#include "bc_block/BlockHeader.h"

#include "bc_status_cache_vote/HeightVoteData.h"

#include "base_timestamp/SystemTimestamp.h"
#include "bc/CodablecashSystemParam.h"
namespace codablecash {

VoteManager::VoteManager(const File* baseDir, const CodablecashSystemParam* config) {
	this->baseDir = baseDir->get(VOTE_HEIGHT_DATA);
	this->mutex = new SysMutex();
	this->cacheManager = new DiskCacheManager();
	this->heightVoteDataIndex = nullptr;

	this->config = config;
}

VoteManager::~VoteManager() {
	close();

	delete this->baseDir;
	delete this->mutex;
	delete this->cacheManager;
}

void VoteManager::initBlank() {
	UnicodeString fileName(VOTE_HEIGHT_DATA);

	BtreeKeyFactory* keyFactory = new BtreeKeyFactory(); __STP(keyFactory);
	HeightVoteDataFactory* dataFactory = new HeightVoteDataFactory(); __STP(dataFactory);

	Btree btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);

	BtreeConfig config;
	config.nodeNumber = 8;
	config.defaultSize = 1024;
	config.blockSize = 32;
	btree.create(&config);
}

void VoteManager::open() {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	UnicodeString fileName(VOTE_HEIGHT_DATA);

	BtreeKeyFactory* keyFactory = new BtreeKeyFactory(); __STP(keyFactory);
	HeightVoteDataFactory* dataFactory = new HeightVoteDataFactory(); __STP(dataFactory);

	this->heightVoteDataIndex = new Btree(this->baseDir, &fileName, this->cacheManager, keyFactory, dataFactory);
	BtreeOpenConfig opconf;
	opconf.numDataBuffer = 256;
	opconf.numNodeBuffer = 512;
	this->heightVoteDataIndex->open(&opconf);
}

void VoteManager::close() noexcept {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	if(this->heightVoteDataIndex != nullptr){
		this->heightVoteDataIndex->close();
		delete this->heightVoteDataIndex, this->heightVoteDataIndex = nullptr;
	}
}

bool VoteManager::registerBlockHeader(const BlockHeader *header, const CodablecashSystemParam *param) {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	uint64_t currentHeight = header->getHeight();

	bool updated = false;
	if(currentHeight > 0){
		HeightVoteData* data = getHeightVoteData(currentHeight); __STP(data);
		const SystemTimestamp* tm = header->getNonceGeneratedtimestamp();

		updated = updateLimitTime(currentHeight, data, tm);
	}

	return updated;
}

bool VoteManager::updateLimitTime(uint64_t height, HeightVoteData *data, const SystemTimestamp *limit) {
	bool updated = false;

	const SystemTimestamp* current = data->getNonceCalculatedTimestamp();
	if(current == nullptr || current->compareTo(limit) > 0){
		ULongKey key(height);

		data->setTimestamp(limit);

		this->heightVoteDataIndex->putData(&key, data);
		updated = true;
	}

	return updated;
}

HeightVoteData* VoteManager::getHeightVoteData(uint64_t height) {
	ULongKey key(height);

	IBlockObject* obj = this->heightVoteDataIndex->findByKey(&key); __STP(obj);
	if(obj == nullptr){
		HeightVoteData* newobj = new HeightVoteData();
		__STP_RESET(obj, newobj);
		obj = newobj;


		this->heightVoteDataIndex->putData(&key, newobj);
	}

	HeightVoteData* ret = dynamic_cast<HeightVoteData*>(__STP_MV(obj));
	assert(ret != nullptr);

	return ret;
}

SystemTimestamp* VoteManager::getPosVoteLimit(uint64_t lastHeight) {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	SystemTimestamp* ret = nullptr;
	uint32_t consensusPosVoteLimitMillis = this->config->getConsensusPosVoteLimitMillis();

	SystemTimestamp mills(0, consensusPosVoteLimitMillis * 1000);

	HeightVoteData* data = getHeightVoteData(lastHeight);
	const SystemTimestamp* tm = data->getNonceCalculatedTimestamp();
	if(tm != nullptr){
		SystemTimestamp lim = *tm + mills;
		ret = dynamic_cast<SystemTimestamp*>(lim.copyData());
	}

	return ret;
}

} /* namespace codablecash */
