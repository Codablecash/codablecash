/*
 * SimnetMainNet.h
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#ifndef SIM_NET_SIMNETMAINNET_H_
#define SIM_NET_SIMNETMAINNET_H_

#include "pubsub/IPubsubCommandExecutor.h"

namespace alinous {
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class ISystemLogger;
class P2pServer;

class SimnetMainNet : public IPubsubCommandExecutor {
public:
	explicit SimnetMainNet(ISystemLogger *logger);
	virtual ~SimnetMainNet();

	void shutdown();

	void setServerNetInfo(int protocol, const UnicodeString* srvhost, int port);
	void startNetwork();


private:
	ISystemLogger *logger;
	P2pServer* p2pServer;

	int protocol;
	UnicodeString* srvhost;
	int port;
};

} /* namespace codablecash */

#endif /* SIM_NET_SIMNETMAINNET_H_ */
