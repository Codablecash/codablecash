/*
 * HeightVoteData.cpp
 *
 *  Created on: 2024/05/01
 *      Author: iizuka
 */

#include "bc_status_cache_vote/HeightVoteData.h"

#include "base_timestamp/SystemTimestamp.h"

namespace codablecash {

HeightVoteData::HeightVoteData(const HeightVoteData &inst) {
	this->timestamp = inst.timestamp != nullptr ? dynamic_cast<SystemTimestamp*>(inst.timestamp->copyData()) : nullptr;
}

HeightVoteData::HeightVoteData() {
	this->timestamp = nullptr;
}

HeightVoteData::~HeightVoteData() {
	delete this->timestamp;
}

int HeightVoteData::binarySize() const {
	int total = sizeof(uint8_t);

	if(this->timestamp != nullptr){
		total += this->timestamp->binarySize();
	}

	return total;
}

void HeightVoteData::toBinary(ByteBuffer *out) const {
	uint8_t bl = this->timestamp != nullptr ? 1 : 0;
	out->put(bl);

	if(this->timestamp != nullptr){
		this->timestamp->toBinary(out);
	}
}

HeightVoteData* HeightVoteData::fromBinary(ByteBuffer *in) {
	HeightVoteData* data = new HeightVoteData();

	uint8_t bl = in->get();
	if(bl > 0){
		data->timestamp = SystemTimestamp::fromBinary(in);
	}

	return data;
}

IBlockObject* HeightVoteData::copyData() const noexcept {
	return new HeightVoteData(*this);
}

void HeightVoteData::setTimestamp(const SystemTimestamp *tm) {
	delete this->timestamp;
	this->timestamp = dynamic_cast<SystemTimestamp*>(tm->copyData());
}


} /* namespace codablecash */
