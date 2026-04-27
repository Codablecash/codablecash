/*
 * AbstractPubSubCommand.h
 *
 *  Created on: 2023/01/07
 *      Author: iizuka
 */

#ifndef PUBSUB_ABSTRACTPUBSUBCOMMAND_H_
#define PUBSUB_ABSTRACTPUBSUBCOMMAND_H_

#include <cstdint>

#include "filestore_block/IBlockObject.h"

namespace alinous {
class ByteBuffer;
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class IClientSocket;
class AbstractCommandResponse;
class IPubsubCommandListner;
class PubSubId;

class AbstractPubSubCommand : public IBlockObject {
public:
	static const constexpr uint32_t TYPE_ECHO{1};
	static const constexpr uint32_t TYPE_END_HANDSHAKE{2};
	static const constexpr uint32_t TYPE_LOGIN{3};
	static const constexpr uint32_t TYPE_CLIENT_LOGIN{4};

	// Client Commands
	static const constexpr uint32_t TYPE_CLIENT_DOWNLOAD_DNS{10};
	static const constexpr uint32_t TYPE_CLIENT_PING{11};
	static const constexpr uint32_t TYPE_CLIENT_TRANSACTION_TRANSFER{12};
	static const constexpr uint32_t TYPE_CLIENT_SYNC_HEADER{13};
	static const constexpr uint32_t TYPE_CLIENT_LIST_STAKING_IDS{14};
	static const constexpr uint32_t TYPE_CLIENT_FETCH_MEMPOOL_TRX{15};
	static const constexpr uint32_t TYPE_CLIENT_FETCH_HEADER_TRX{16};

	// DNS Commands
	static const constexpr uint32_t TYPE_NETWORK_NODE_SHUTDOWN{100};

	// Inter Nodes Commands
	static const constexpr uint32_t TYPE_NODE_NOP{300};
	static const constexpr uint32_t TYPE_NODE_SEND_TRANSACTION{301};
	static const constexpr uint32_t TYPE_NODE_SYNC_MEMPOOL{302};
	static const constexpr uint32_t TYPE_NODE_SYNC_HEADER{303};
	static const constexpr uint32_t TYPE_NODE_DOWNLOAD_BLOCK{304};
	static const constexpr uint32_t TYPE_NODE_REPORT_MINED_BLOCK{305};
	static const constexpr uint32_t TYPE_NODE_DOWNLOAD_OMITTED_BLOCK_BODY{306};
	static const constexpr uint32_t TYPE_NODE_DOWNLOAD_TRANSACTIONS{307};

	// Consensus Commands
	static const constexpr uint32_t TYPE_CONSENSUS_REPORT_NONCE_CALCULATED{350};
	static const constexpr uint32_t TYPE_CONSENSUS_SEND_VOTE_TRANSACTION{351};

	// Client Notify Commands
	static const constexpr uint32_t TYPE_CLIENT_NOTIFY_NEW_TRANSACTION{401};
	static const constexpr uint32_t TYPE_CLIENT_NOTIFY_BLOCK_MINED{402};

	// Mining Pool
	static const constexpr uint32_t TYPE_POW_POOL_NOTIFY_DATA_CHANGED{500};

	// Mining Pool Client Request
	static const constexpr uint32_t TYPE_POW_POOL_CLIENT_STATUS{550};
	static const constexpr uint32_t TYPE_POW_POOL_CLIENT_CHECK_DATA{551};
	static const constexpr uint32_t TYPE_POW_POOL_CLIENT_NONCE_CALCULATED{552};


	AbstractPubSubCommand(const AbstractPubSubCommand& inst);
	explicit AbstractPubSubCommand(uint32_t type);
	virtual ~AbstractPubSubCommand();

	static AbstractPubSubCommand* createFromBinary(ByteBuffer* buff);

	virtual void fromBinary(ByteBuffer* buff) = 0;
	virtual int binarySize() const = 0;
	virtual void toBinary(ByteBuffer* buff) const = 0;

	/**
	 * IPubsubCommandListner#getExecutor() is Instance
	 */
	virtual AbstractCommandResponse* execute(const PubSubId* pubsubId, IPubsubCommandListner* listner) const = 0;

	uint32_t getType() const noexcept {
		return type;
	}

protected:
	void putString(ByteBuffer* out, const UnicodeString* str) const noexcept;
	UnicodeString* getString(ByteBuffer* in) const noexcept;
	int stringSize(const UnicodeString* str) const noexcept;

protected:
	uint32_t type;
};

} /* namespace codablecash */

#endif /* PUBSUB_ABSTRACTPUBSUBCOMMAND_H_ */
