/*
 * test_p2p_client_command.cpp
 *
 *  Created on: 2023/09/24
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"

#include "../../blockchain/utils/InstanceDriver.h"
#include "../../blockchain/utils/ClientConnectUtils.h"

#include "../../test_utils/TestPortSelection.h"

#include "base_io/File.h"

#include "bc/DebugDefaultLogger.h"

#include "bc_block_generator/MiningConfig.h"

#include "ipconnect/IpV6ClientConnection.h"

#include "bc_p2p_client/P2pClient.h"

#include "bc_base/BalanceUnit.h"

#include "bc_p2p_cmd_node/InvalidTransactionException.h"

#include "bc/CodablecashNodeInstance.h"
#include "dummy/DummyClientListner.h"
#include "osenv/funcs.h"

#include "bc_status_cache/BlockchainController.h"

#include "data_history_data/DataStructureException.h"

#include "base/StackRelease.h"

#include "base_thread/StackUnlocker.h"

#include "base_thread/SysMutex.h"
#include "bc/CodablecashSystemParam.h"

#include "../../blockchain/utils/DebugCodablecashSystemParamSetup.h"
using alinous::Os;

using namespace codablecash;

TEST_GROUP(TestP2pCommandGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestP2pCommandGroup, case01){
	File projectFolder = this->env->testCaseDir();
	InstanceDriver driver(&projectFolder);

	int ticketNum = 30;

	StackTestPortGetter portSel;
	int port = portSel.allocPort();

	CodablecashSystemParam config;
	DebugCodablecashSystemParamSetup::setupConfig01(config);
	config.setPowBlockTimeMills(5);

	driver.initWallet(1);
	driver.initInstance(&config);

	driver.startV6Listner(port);

	MiningConfig mconfig;
	driver.startMiner(&mconfig);
	driver.startFinalize();

	driver.suspendMiner();

	driver.initGenesisBalance(0, 10000L * 1000L * 1000L);

	DummyClientListner listner;
	{
		DebugDefaultLogger* logger = driver.getLogger();
		logger->addSection(ISystemLogger::DEBUG_NODE_TRANSFER_RESPONSE);

		UnicodeString strLocal(L"::1");
		P2pClient* client = ClientConnectUtils::connect(&strLocal, port, 0, logger); __STP(client);
		client->addListner(&listner);

		driver.registerVotingNode(client);

		driver.resumeMiner();
		driver.waitUntilVotingNodeRegistered();
		driver.suspendMiner();

		{
			StackUnlocker __lock(logger->getMutex(), __FILE__, __LINE__);
			printf("susupend miner\n");
		}

		MemoryPool* pool = driver.getInstance()->getMemoryPool();

		int maxLoop = ticketNum;
		for(int i = 0; i != maxLoop; ++i) {
			BalanceUnit stakeAmount(1000L);

			try{
				driver.registerTicket(client, stakeAmount, pool);
				{
					StackUnlocker __lock(logger->getMutex(), __FILE__, __LINE__);
					printf("register ticket %d\n", i);
				}
			}
			catch(Exception* e){
				__STP(e);
				const UnicodeString* message = e->getMessage();
				const char* cstr = message->toCString();
				StackArrayRelease<const char> __ar(cstr);

				{
					StackUnlocker __lock(logger->getMutex(), __FILE__, __LINE__);
					printf("register ticket error %s\n", cstr);
				}
			}
		}

		{
			BlockchainController* ctrl = driver.getInstance()->getController();
			int count = ctrl->getMempoolTrxCount();

			{
				StackUnlocker __lock(logger->getMutex(), __FILE__, __LINE__);
				printf("count %d\n", count);
			}

			while(count <= ticketNum){
				Os::usleep(50);

				{
					StackUnlocker __lock(logger->getMutex(), __FILE__, __LINE__);
					printf("count %d\n", count);
				}

				count = ctrl->getMempoolTrxCount();
			}
		}

		{
			StackUnlocker __lock(logger->getMutex(), __FILE__, __LINE__);
			printf("count getMempoolTrxCount()\n");
		}
	}

	driver.resumeMiner();

	// driver.waitUntilHeight(10);
	driver.waitUntilFinalizedHeight(5);

	int size = listner.size();
	CHECK(size == ticketNum + 1)

}

//
TEST(TestP2pCommandGroup, InvalidTransactionException01){
	testException<InvalidTransactionException>();
	testException<DataStructureException>();
}
