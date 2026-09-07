/*
 * SimnetClient.h
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#ifndef SIM_NET_CLIENT_SIMNETCLIENT_H_
#define SIM_NET_CLIENT_SIMNETCLIENT_H_

#include "pubsub/IPubsubCommandExecutor.h"

namespace alinous {
class UnicodeString;
}
using namespace alinous;


namespace codablecash {

class ISystemLogger;
class P2pHandshake;
class SimnetPubsubCommandExecutor;

class SimnetClient : public IPubsubCommandExecutor {
public:
	explicit SimnetClient(ISystemLogger* logger);
	virtual ~SimnetClient();

	void start(int protocol, const UnicodeString *host, int port);
	void shutdown();

private:
	ISystemLogger* logger;

	P2pHandshake *handshake;
	SimnetPubsubCommandExecutor* exec;
};

} /* namespace codablecash */

#endif /* SIM_NET_CLIENT_SIMNETCLIENT_H_ */
