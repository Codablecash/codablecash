/*
 * AbstractCommandResponse.h
 *
 *  Created on: 2023/01/13
 *      Author: iizuka
 */

#ifndef PUBSUB_ABSTRACTCOMMANDRESPONSE_H_
#define PUBSUB_ABSTRACTCOMMANDRESPONSE_H_

#include <cstdint>

namespace alinous {
class ByteBuffer;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class IClientSocket;

class AbstractCommandResponse {
public:
	static const constexpr uint32_t TYPE_RES_ECHO{1};
	static const constexpr uint32_t TYPE_RES_OK{2};
	static const constexpr uint32_t TYPE_RES_WARNING{3};
	static const constexpr uint32_t TYPE_RES_ERROR{4};
	static const constexpr uint32_t TYPE_RES_SIGNATURE_ERROR{5};
	static const constexpr uint32_t TYPE_RES_LOGIN_ERROR{6};

	static const constexpr uint32_t TYPE_RES_PING{10};
	static const constexpr uint32_t TYPE_RES_DOWNLOAD_DNS_INFO{11};
	static const constexpr uint32_t TYPE_RES_CLIENT_LIST_STAKING_IDS{12};
	static const constexpr uint32_t TYPE_RES_CLIENT_FETCH_MEMPOOL_TRX{13};
	static const constexpr uint32_t TYPE_RES_CLIENT_FETCH_HEADER_TRX{14};

	static const constexpr uint32_t TYPE_NODE_COMMAND_VALIDATION_ERROR{99};
	static const constexpr uint32_t TYPE_RES_SYNC_MEMPOOL{100};
	static const constexpr uint32_t TYPE_RES_SYNC_HEADER{101};
	static const constexpr uint32_t TYPE_RES_DOWNLOAD_BLOCK{102};
	static const constexpr uint32_t TYPE_RES_DOWNLOAD_OMITTED_BLOCK_BODY{103};
	static const constexpr uint32_t TYPE_RES_DOWNLOAD_TRANSACTIONS{104};

	static const constexpr uint32_t TYPE_RES_CLIENT_SYNC_HEADER{110};


	static const constexpr uint32_t TYPE_RES_POW_POOL_CLIENT_STATUS{201};
	static const constexpr uint32_t TYPE_RES_POW_POOL_CLIENT_CHECK_DATA{202};

	explicit AbstractCommandResponse(uint32_t type);
	virtual ~AbstractCommandResponse();

	static AbstractCommandResponse* createFromBinary(ByteBuffer* buff);

	virtual void fromBinary(ByteBuffer* buff) = 0;
	virtual int binarySize() const = 0;
	virtual void toBinary(ByteBuffer* buff) const = 0;

	virtual void sendCommand(IClientSocket* client);

	uint32_t getType() const noexcept {
		return type;
	}

	virtual UnicodeString* toString() const noexcept = 0;

protected:
	uint32_t type;
};

} /* namespace codablecash */

#endif /* PUBSUB_ABSTRACTCOMMANDRESPONSE_H_ */
