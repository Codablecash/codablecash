/*
 * AbstractCommandResponse.cpp
 *
 *  Created on: 2023/01/13
 *      Author: iizuka
 */

#include "pubsub_cmd/AbstractCommandResponse.h"

#include "pubsub_cmd/EchoPubSubCommandResponse.h"
#include "pubsub_cmd/OkPubsubResponse.h"
#include "pubsub_cmd/WarningResponse.h"
#include "pubsub_cmd/ErrorPubsubResponse.h"

#include "bc_p2p_cmd/SignatureErrorResponse.h"
#include "bc_p2p_cmd/LoginErrorResponse.h"

#include "bc_p2p_cmd_client/PingNodeCommandResponse.h"
#include "bc_p2p_cmd_client/DownloadDnsInfoCommandResponse.h"
#include "bc_p2p_cmd_client/ClientListStakingNodeIdsCommandResponse.h"
#include "bc_p2p_cmd_client/ClientFetchMempoolTrxCommandResponse.h"
#include "bc_p2p_cmd_client/ClientFetchHeaderTransactionsCommandResponse.h"

#include "bc_p2p_cmd_node/SyncMempoolNodeCommandResponse.h"
#include "bc_p2p_cmd_node/SyncHeaderNodeCommandResponse.h"
#include "bc_p2p_cmd_node/DownloadBlockNodeCommandResponse.h"
#include "bc_p2p_cmd_node/DownloadOmittedBlockBodyNodeCommandResponse.h"
#include "bc_p2p_cmd_node/DownloadTransactionsNodeCommandResponse.h"
#include "bc_p2p_cmd_node/NodeCommandValidationErrorResponse.h"

#include "bc_p2p_cmd_client/ClientSyncHeaderCommandResponse.h"

#include "pubsub/PubsubCommandException.h"

#include "base_io/ByteBuffer.h"

#include "base/StackRelease.h"

#include "ipconnect/IClientSocket.h"

#include "pow_pool_client_cmd/PoWPoolStatusCommandResponse.h"
#include "pow_pool_client_cmd/PoWPoolCheckDataCommandResponse.h"

namespace codablecash {

AbstractCommandResponse::AbstractCommandResponse(uint32_t type) {
	this->type = type;
}

AbstractCommandResponse::~AbstractCommandResponse() {

}

AbstractCommandResponse* AbstractCommandResponse::createFromBinary(ByteBuffer *buff) {
	AbstractCommandResponse* ret = nullptr;

	uint32_t type = buff->getInt();
	switch(type){
	case TYPE_RES_ECHO:
		ret = new EchoPubSubCommandResponse();
		break;
	case TYPE_RES_OK:
		ret = new OkPubsubResponse();
		break;
	case TYPE_RES_WARNING:
		ret = new WarningResponse();
		break;
	case TYPE_RES_ERROR:
		ret = new ErrorPubsubResponse();
		break;
	case TYPE_RES_SIGNATURE_ERROR:
		ret = new SignatureErrorResponse();
		break;
	case TYPE_RES_LOGIN_ERROR:
		ret = new LoginErrorResponse();
		break;
	case TYPE_RES_PING:
		ret = new PingNodeCommandResponse();
		break;
	case TYPE_RES_DOWNLOAD_DNS_INFO:
		ret = new DownloadDnsInfoCommandResponse();
		break;
	case TYPE_RES_CLIENT_LIST_STAKING_IDS:
		ret = new ClientListStakingNodeIdsCommandResponse();
		break;
	case TYPE_RES_CLIENT_FETCH_MEMPOOL_TRX:
		ret = new ClientFetchMempoolTrxCommandResponse();
		break;
	case TYPE_RES_CLIENT_FETCH_HEADER_TRX:
		ret = new ClientFetchHeaderTransactionsCommandResponse();
		break;
	case TYPE_NODE_COMMAND_VALIDATION_ERROR:
		ret = new NodeCommandValidationErrorResponse();
		break;
	case TYPE_RES_SYNC_MEMPOOL:
		ret = new SyncMempoolNodeCommandResponse();
		break;
	case TYPE_RES_SYNC_HEADER:
		ret = new SyncHeaderNodeCommandResponse();
		break;
	case TYPE_RES_DOWNLOAD_BLOCK:
		ret = new DownloadBlockNodeCommandResponse();
		break;
	case TYPE_RES_DOWNLOAD_OMITTED_BLOCK_BODY:
		ret = new DownloadOmittedBlockBodyNodeCommandResponse();
		break;
	case TYPE_RES_DOWNLOAD_TRANSACTIONS:
		ret = new DownloadTransactionsNodeCommandResponse();
		break;
	case TYPE_RES_CLIENT_SYNC_HEADER:
		ret = new ClientSyncHeaderCommandResponse();
		break;

	case TYPE_RES_POW_POOL_CLIENT_STATUS:
		ret = new PoWPoolStatusCommandResponse();
		break;
	case TYPE_RES_POW_POOL_CLIENT_CHECK_DATA:
		ret = new PoWPoolCheckDataCommandResponse();
		break;
	default:
		throw new PubsubCommandException(L"Wrong command response type.", __FILE__, __LINE__);
	}

	ret->fromBinary(buff);

	return ret;
}

void AbstractCommandResponse::sendCommand(IClientSocket *client) {
	int size = binarySize();
	int length = size + sizeof(uint32_t);

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(length, true); __STP(buff);
	buff->putInt(size);

	toBinary(buff);

	buff->position(0);
	const uint8_t* ar = buff->array();

	client->write((const char*)ar, length);
}

} /* namespace codablecash */
