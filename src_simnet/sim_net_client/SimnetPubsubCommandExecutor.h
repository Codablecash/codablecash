/*
 * SimnetPubsubCommandExecutor.h
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#ifndef SIM_NET_CLIENT_SIMNETPUBSUBCOMMANDEXECUTOR_H_
#define SIM_NET_CLIENT_SIMNETPUBSUBCOMMANDEXECUTOR_H_

#include "pubsub/IPubsubCommandListner.h"

namespace codablecash {

class SimnetClient;

class SimnetPubsubCommandExecutor : public IPubsubCommandListner {
public:
	explicit SimnetPubsubCommandExecutor(SimnetClient* client);
	virtual ~SimnetPubsubCommandExecutor();

	virtual void fireOnExeptionOnSubscriber(
			const codablecash::PubSubId *pubsubId, const alinous::Exception *e);
	virtual void fireExecuteCommand(const codablecash::PubSubId *pubsubId,
			const codablecash::AbstractPubSubCommand *cmd);
	virtual void fireOnSubscribeEnds(const codablecash::PubSubId *pubsubId);
	virtual IPubsubCommandExecutor* getExecutor() const noexcept;

private:
	SimnetClient* client;
};

} /* namespace codablecash */

#endif /* SIM_NET_CLIENT_SIMNETPUBSUBCOMMANDEXECUTOR_H_ */
