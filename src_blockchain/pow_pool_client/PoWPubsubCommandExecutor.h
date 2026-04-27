/*
 * PoWPubsubCommandExecutor.h
 *
 *  Created on: Apr 18, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_POWPUBSUBCOMMANDEXECUTOR_H_
#define POW_POOL_CLIENT_POWPUBSUBCOMMANDEXECUTOR_H_

#include "pubsub/IPubsubCommandListner.h"


namespace codablecash {

class PoWWorkerClient;

class PoWPubsubCommandExecutor : public IPubsubCommandListner {
public:
	explicit PoWPubsubCommandExecutor(PoWWorkerClient* client);
	virtual ~PoWPubsubCommandExecutor();

	virtual void fireExecuteCommand(const PubSubId* pubsubId, const AbstractPubSubCommand* cmd);
	virtual void fireOnExeptionOnSubscriber(const PubSubId* pubsubId, const Exception* e);
	virtual void fireOnSubscribeEnds(const PubSubId* pubsubId);

	virtual IPubsubCommandExecutor* getExecutor() const noexcept;

private:
	PoWWorkerClient* client;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_POWPUBSUBCOMMANDEXECUTOR_H_ */
