/*
 * test_instance_space_move.cpp
 *
 *  Created on: Mar 14, 2026
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

#include "ext_arguments/NumericArgument.h"

#include "engine/sc_analyze/AnalyzedType.h"

#include "bc_block/BlockHeaderId.h"
using namespace codablecash;

TEST_GROUP(TestBuildInstanceSpaceGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};


TEST(TestBuildInstanceSpaceGroup, case01){
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
	{ // test no module address
		ModularSmartcontractExecutor* executor = exec.getExecutor();

		// public key and module address
		HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
		BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
		ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);
		ScPublicKey publicKey = privateKey->generatePublicKey();

		SmartcontractModuleAddress* moduleAddress = SmartcontractModuleAddress::createAddress(0, &publicKey); __STP(moduleAddress);
		{
			UnicodeString* str = moduleAddress->toString(); __STP(str);
			const char* log  = str->toCString();
			::printf("%s\n", log);
			delete [] log;
		}

		SmartcontractProjectId* id = executor->getLatestProjectId(moduleAddress);
		CHECK(id == nullptr);
	}

	{
		UnicodeString module(L"exec");
		UnicodeString methodName(L"setCount");

		ArrayList<AbstractFunctionExtArguments> args;
		args.setDeleteOnExit();
		args.addElement(new NumericArgument(11, AnalyzedType::TYPE_INT));

		BlockHeaderId* execHeaderId = BlockHeaderId::makeRandomHeaderId(); __STP(execHeaderId);
		uint64_t execHeight = 12;

		exec.process(address, &module, &methodName, &args, execHeaderId, execHeight);
	}
}


