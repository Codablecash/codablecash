/*
 * test_simnet_client.cpp
 *
 *  Created on: Sep 3, 2026
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"

#include "../test_utils/TestPortSelection.h"

#include "sim_net/SimnetMainNet.h"

#include "sim_net_client/SimnetClient.h"

#include "bc/DebugDefaultLogger.h"

#include "bc_p2p_info/P2pNodeRecord.h"


using namespace codablecash;

TEST_GROUP(TestSimnetClientGroup) {
	TEST_SETUP(){this->env->setup();}
	TEST_TEARDOWN(){this->env->teardown();}
};

TEST(TestSimnetClientGroup, case01){
	StackTestPortGetter portSel;
	int port = portSel.allocPort();

	DebugDefaultLogger logger;
	logger.addSection(ISystemLogger::DEBUG_POOL_TRANSFER_RESPONSE);

	SimnetMainNet* net = new SimnetMainNet(&logger); __STP(net);

	UnicodeString srvhost(L"127.0.0.1");
	net->setServerNetInfo(P2pNodeRecord::TCP_IP_V4, &srvhost, port);
	net->startNetwork();

	{
		SimnetClient client(&logger);
		UnicodeString host(L"127.0.0.1");
		client.start(P2pNodeRecord::TCP_IP_V4, &host, port);
	}

}
