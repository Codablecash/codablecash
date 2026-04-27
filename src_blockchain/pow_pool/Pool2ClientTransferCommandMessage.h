/*
 * Pool2ClientTransferCommandMessage.h
 *
 *  Created on: Apr 23, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_POOL2CLIENTTRANSFERCOMMANDMESSAGE_H_
#define POW_POOL_POOL2CLIENTTRANSFERCOMMANDMESSAGE_H_

#include "bc_p2p_processor/AbstractNetworkTransferCommandMessage.h"

namespace codablecash {

class PubSubId;
class AbstractPoWClientNotifyCommand;

class Pool2ClientTransferCommandMessage : public AbstractNetworkTransferCommandMessage {
public:
	Pool2ClientTransferCommandMessage(const PubSubId *pusubId, const AbstractPoWClientNotifyCommand* command);
	virtual ~Pool2ClientTransferCommandMessage();

	virtual void execute(ICommandParameter* param);

private:
	PubSubId *pusubId;
	AbstractPoWClientNotifyCommand* command;
};

} /* namespace codablecash */

#endif /* POW_POOL_POOL2CLIENTTRANSFERCOMMANDMESSAGE_H_ */
