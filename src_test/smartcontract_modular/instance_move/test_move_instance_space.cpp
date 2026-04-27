/*
 * test_move_instance_space.cpp
 *
 *  Created on: Mar 25, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../utils/ModularProjectUtils.h"

#include "modular_project/ModularSmartcontractProject.h"

#include "MoveTestUtil.h"

#include "transaction/SmartcontractInstanceAddress.h"
#include "transaction/SmartcontractModuleAddress.h"

#include "bc_wallet/HdWalletSeed.h"

#include "ecda/ScPrivateKey.h"
#include "ecda/ScPublicKey.h"

#include "smartcontract_executor/ModularSmartcontractExecutor.h"
#include "smartcontract_executor/SmartcontractExecContextRegistory.h"

#include "smartcontract_executor_cursor/SmartcontractCallSequenceBuilder.h"
#include "smartcontract_executor_cursor/SmartcontractCallSequence.h"
#include "smartcontract_executor_cursor/SessionContextCursorData.h"

#include "smartcontract_executor_index/InstanceSessionContext.h"

#include "ext_arguments/NumericArgument.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "bc_block/BlockHeaderId.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "smartcontract_executor_cursor/SmartcontractContextMover.h"

#include "smartcontract_cache/InstanceSpaceReleaser.h"
#include "smartcontract_cache/InstanceSpace.h"

#include "bc/DebugDefaultLogger.h"
using namespace codablecash;

TEST_GROUP(TestMoveInstanceSpaceGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};

TEST(TestMoveInstanceSpaceGroup, case01){
	DebugDefaultLogger logger;
	// logger.setSection(ISystemLogger::DEBUG_DBSESSION_INFO);

	const File* projectFolder = this->env->getProjectRoot();
	ModularProjectUtils utils(L"src_test/smartcontract_modular/instance_move/resources/case01/project01/", projectFolder);
	const File* baseDir = utils.getFolder();

	ModularSmartcontractProject project(baseDir);
	project.loadProject();

	File projectTestFolder = this->env->testCaseDir();
	_ST(File, baseTestDir, projectTestFolder.get(L"exec"))

	MoveTestUtil exec(&project, baseTestDir);
	exec.init();

	exec.registerProject();

	SmartcontractInstanceAddress* address = SmartcontractInstanceAddress::createAddress(0); __STP(address);
	exec.createInstance(address, 11);

	int maxLoop = 5;
	for(int i = 0; i != maxLoop; ++i) {
		UnicodeString module(L"exec");
		UnicodeString methodName(L"setCount");

		ArrayList<AbstractFunctionExtArguments> args;
		args.setDeleteOnExit();
		args.addElement(new NumericArgument(11 + i, AnalyzedType::TYPE_INT));

		BlockHeaderId* execHeaderId = BlockHeaderId::makeRandomHeaderId(); __STP(execHeaderId);
		uint64_t execHeight = 12 + i;

		exec.process(address, &module, &methodName, &args, execHeaderId, execHeight);
	}

	ArrayList<InstanceSessionContext>* list = exec.getHeads(address); __STP(list);
	list->setDeleteOnExit();
	CHECK(list->size() == 1);

	InstanceSessionContext* ctx = list->get(0);
	const CdbDatabaseSessionId* trxId = ctx->getSessionId();

	ModularSmartcontractExecutor* executor = exec.getExecutor();
	SmartcontractExecContextRegistory* contextReg = executor->getSmartcontractExecContextRegistory();

	SmartcontractCallSequenceBuilder builder(contextReg);
	builder.build(trxId);

	const SmartcontractCallSequence* sequence = builder.getSequence();

	int seqSize = sequence->size();
	CHECK(seqSize == maxLoop + 1);

	int forkPos = 2;
	SessionContextCursorData* data = sequence->get(forkPos);
	CdbDatabaseSessionId* lastTrxId = data->getSessionId();
	lastTrxId = dynamic_cast<CdbDatabaseSessionId*>(lastTrxId->copyData());

	// pos 3 serial 4
	maxLoop = sequence->size() + 1;
	for(int i = forkPos + 1; i != maxLoop; ++i)
	{
		UnicodeString module(L"exec");
		UnicodeString methodName(L"setCount");

		ArrayList<AbstractFunctionExtArguments> args;
		args.setDeleteOnExit();
		args.addElement(new NumericArgument(13, AnalyzedType::TYPE_INT));

		BlockHeaderId* execHeaderId = BlockHeaderId::makeRandomHeaderId(); __STP(execHeaderId);
		uint64_t execHeight = 12 + forkPos + i;

		CdbDatabaseSessionId* sessionId = exec.process(address, &module, &methodName, &args, execHeaderId, execHeight, lastTrxId);

		delete lastTrxId;
		lastTrxId = sessionId;
	}
	delete lastTrxId;

	ArrayList<InstanceSessionContext>* list2 = exec.getHeads(address); __STP(list2);
	list2->setDeleteOnExit();
	CHECK(list2->size() == 2);


	///////////////////////////
	SmartcontractCallSequenceBuilder builder2(contextReg);
	const CdbDatabaseSessionId* destTrxId = nullptr;

	maxLoop = list2->size();
	for(int i = 0; i != maxLoop; ++i){
		InstanceSessionContext* ctx2 = list2->get(i);

		const CdbDatabaseSessionId* id = ctx2->getSessionId();
		if(!id->equals(trxId)){
			builder2.build(id);

			destTrxId = id;
		}
	}

	const SmartcontractCallSequence* sequence2 = builder2.getSequence();

	bool bl = sequence->isSamechain(sequence2);
	CHECK(bl == false);

	const SessionContextCursorData* common = sequence->getCommonPoint(sequence2);
	CHECK(common != nullptr);
	CHECK(common->getCallSerial() == forkPos + 1); // forkPos starts from 0, serial starts from 1


	///////////////////////// Move
	{
		SmartcontractContextMover mover(executor, address, trxId, destTrxId);
		mover.execute(&logger);

		InstanceSpace* space = executor->loadFromCache(address);
		InstanceSpaceReleaser __space(space);

		const CdbDatabaseSessionId* id = space->getDatabaseSessionId();
		bool bl = id->equals(destTrxId);
		CHECK(bl == true);

		sequence->logSequence(&logger);
		sequence2->logSequence(&logger);

		// sequence check
		{
			SmartcontractCallSequenceBuilder __builder(contextReg);
			__builder.build(destTrxId);

			const SmartcontractCallSequence* __seq = __builder.getSequence();
			__seq->logSequence(&logger);
		}
	}
	{
		// back
		SmartcontractContextMover mover(executor, address, destTrxId, trxId);
		mover.execute(&logger);

		InstanceSpace* space = executor->loadFromCache(address);
		InstanceSpaceReleaser __space(space);

		const CdbDatabaseSessionId* id = space->getDatabaseSessionId();
		bool bl = id->equals(trxId);
		CHECK(bl == true);
	}

	int testPos = 3;
	{
		SessionContextCursorData* data3 = sequence->get(testPos);
		const CdbDatabaseSessionId* trxId3 = data3->getSessionId();

		SmartcontractContextMover mover(executor, address, trxId, trxId3);
		mover.execute(&logger);

		InstanceSpace* space = executor->loadFromCache(address);
		InstanceSpaceReleaser __space(space);

		const CdbDatabaseSessionId* id = space->getDatabaseSessionId();
		bool bl = id->equals(trxId3);
		CHECK(bl == true);
	}
	{
		SessionContextCursorData* data3 = sequence->get(testPos);
		const CdbDatabaseSessionId* trxId3 = data3->getSessionId();

		SmartcontractContextMover mover(executor, address, trxId3, trxId);
		mover.execute(&logger);

		InstanceSpace* space = executor->loadFromCache(address);
		InstanceSpaceReleaser __space(space);

		const CdbDatabaseSessionId* id = space->getDatabaseSessionId();
		bool bl = id->equals(trxId);
		CHECK(bl == true);
	}
}



