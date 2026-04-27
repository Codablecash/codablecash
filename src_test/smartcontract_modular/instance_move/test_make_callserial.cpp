/*
 * test_make_callserial.cpp
 *
 *  Created on: Mar 20, 2026
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

#include "ext_arguments/NumericArgument.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "bc_block/BlockHeaderId.h"

#include "smartcontract_executor_index/InstanceSessionContext.h"

#include "smartcontract_executor_cursor/SmartcontractCallSequenceBuilder.h"


using namespace codablecash;

TEST_GROUP(TestMakecallSerialGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};

TEST(TestMakecallSerialGroup, case01){
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

	int maxLoop = 10;
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


}

