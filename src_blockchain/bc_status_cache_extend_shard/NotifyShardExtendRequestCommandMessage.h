/*
 * NotifyShardExtendRequestCommandMessage.h
 *
 *  Created on: Jul 5, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_EXTEND_SHARD_NOTIFYSHARDEXTENDREQUESTCOMMANDMESSAGE_H_
#define BC_STATUS_CACHE_EXTEND_SHARD_NOTIFYSHARDEXTENDREQUESTCOMMANDMESSAGE_H_

#include "bc_processor/AbstractCentralProcessorCommandMessage.h"
#include <cstdint>

namespace codablecash {

class BlockHeaderId;

class NotifyShardExtendRequestCommandMessage : public AbstractCentralProcessorCommandMessage {
public:
	NotifyShardExtendRequestCommandMessage();
	virtual ~NotifyShardExtendRequestCommandMessage();

	void setNewShardZone(uint16_t newShardZone) noexcept;
	void setRequestingZone(uint16_t requestingZone) noexcept;
	void setHeaderInfo(uint64_t height, const BlockHeaderId* headerId) noexcept;

protected:
	virtual void process(CentralProcessor* processor);

private:
	uint16_t newShardZone;
	uint16_t requestingZone;

	uint64_t height;
	BlockHeaderId* headerId;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_EXTEND_SHARD_NOTIFYSHARDEXTENDREQUESTCOMMANDMESSAGE_H_ */
