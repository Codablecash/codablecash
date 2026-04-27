/*
 * HeightVoteData.h
 *
 *  Created on: 2024/05/01
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_VOTE_HEIGHTVOTEDATA_H_
#define BC_STATUS_CACHE_VOTE_HEIGHTVOTEDATA_H_

#include "filestore_block/IBlockObject.h"

#include "base/ArrayList.h"

namespace alinous {
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {


class HeightVoteData : public IBlockObject {
public:
	HeightVoteData(const HeightVoteData& inst);
	HeightVoteData();
	virtual ~HeightVoteData();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static HeightVoteData* fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	void setTimestamp(const SystemTimestamp* tm);

	const SystemTimestamp* getNonceCalculatedTimestamp() const noexcept {
		return this->timestamp;
	}


private:
	SystemTimestamp* timestamp;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_VOTE_HEIGHTVOTEDATA_H_ */
