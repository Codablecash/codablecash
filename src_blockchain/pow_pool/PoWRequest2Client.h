/*
 * PoWRequest2Client.h
 *
 *  Created on: Apr 20, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_POWREQUEST2CLIENT_H_
#define POW_POOL_POWREQUEST2CLIENT_H_

namespace alinous {
class ByteBuffer;
class BigInteger;
}
using namespace alinous;

namespace codablecash {

class PoWRequestData;

class PoWRequest2Client {
public:
	PoWRequest2Client(const PoWRequest2Client& inst);
	PoWRequest2Client();
	virtual ~PoWRequest2Client();

	int binarySize() const;
	void toBinary(ByteBuffer* out) const;
	static PoWRequest2Client* fromBinary(ByteBuffer* in);

	void setRequestData(const PoWRequestData* data);
	void setSolt(const BigInteger* solt);
	void setSuspended(bool bl);

	PoWRequestData* getRequestData() const noexcept {
		return this->data;
	}
	BigInteger* getSolt() const noexcept {
		return solt;
	}
	bool isWaiting() const noexcept {
		return this->waiting;
	}

private:
	bool waiting;
	PoWRequestData* data;
	BigInteger* solt;
};

} /* namespace codablecash */

#endif /* POW_POOL_POWREQUEST2CLIENT_H_ */
