/*
 * HeightVoteDataFactory.cpp
 *
 *  Created on: 2024/07/23
 *      Author: iizuka
 */

#include "bc_status_cache_vote/HeightVoteDataFactory.h"
#include "bc_status_cache_vote/HeightVoteData.h"

#include "btree/BtreeStorage.h"
#include "btree/DataNode.h"


namespace codablecash {

HeightVoteDataFactory::HeightVoteDataFactory() {

}

HeightVoteDataFactory::~HeightVoteDataFactory() {

}

IBlockObject* HeightVoteDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return HeightVoteData::fromBinary(in);
}

void HeightVoteDataFactory::registerData(const AbstractBtreeKey *key, const IBlockObject *data, DataNode *dataNode,	BtreeStorage *store) const {
	uint64_t dataFpos = dataNode->getDataFpos();
	if(dataFpos != 0){
		store->removeData(dataFpos);
	}

	dataFpos = store->storeData(data);
	dataNode->setDataFpos(dataFpos);
}

bool HeightVoteDataFactory::beforeRemove(DataNode *dataNode, BtreeStorage *store, const AbstractBtreeKey *key) const {
	return true;
}

AbstractBtreeDataFactory* HeightVoteDataFactory::copy() const noexcept {
	return new HeightVoteDataFactory();
}

} /* namespace codablecash */
