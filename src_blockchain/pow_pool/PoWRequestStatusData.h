/*
 * PoWRequestStatusData.h
 *
 *  Created on: Apr 20, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_POWREQUESTSTATUSDATA_H_
#define POW_POOL_POWREQUESTSTATUSDATA_H_

namespace alinous {
class ByteBuffer;
}
using namespace alinous;

namespace codablecash {

class PoWRequestData;

class PoWRequestStatusData {
public:
	PoWRequestStatusData(const PoWRequestStatusData& inst);
	PoWRequestStatusData();
	virtual ~PoWRequestStatusData();

	void setWaiting(bool bl);
	bool isWaiting() const noexcept {
		return this->waiting;
	}

	int binarySize() const;
	void toBinary(ByteBuffer* out) const;
	static PoWRequestStatusData* fromBinary(ByteBuffer* in);

private:
	bool waiting;
};

} /* namespace codablecash */

#endif /* POW_POOL_POWREQUESTSTATUSDATA_H_ */
