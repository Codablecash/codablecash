/*
 * test_create_exec_operation.cpp
 *
 *  Created on: Jan 22, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "../utils/ModularProjectUtils.h"

#include "modular_project/ModularSmartcontractProject.h"
#include "modular_project/SmartcontractProjectId.h"

#include "modular_project_registory/SmartcontractProjectData.h"

#include "smartcontract_executor/ModularSmartcontractExecutor.h"
#include "smartcontract_executor/SmartcontractExecResult.h"

#include "transaction/SmartcontractInstanceAddress.h"
#include "transaction/SmartcontractModuleAddress.h"

#include "smartcontract_cache/InstanceSpaceReleaser.h"
#include "smartcontract_cache/InstanceSpace.h"

#include "smartcontract_instance/ModularSmartcontractInstance.h"

#include "base/ArrayList.h"

#include "ext_arguments/AbstractFunctionExtArguments.h"

#include "ext_binary/AbstractExtObject.h"

#include "smartcontract_executor_operations/CreateSmartcontractInstanceOperation.h"
#include "smartcontract_executor_operations/RegisterSmartcontractProjectOperation.h"

#include "bc_block/BlockHeaderId.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "bc_wallet/HdWalletSeed.h"

#include "numeric/BigInteger.h"

#include "ecda/ScPrivateKey.h"
#include "ecda/ScPublicKey.h"

#include "smartcontract_executor_operations/ProcessSmartcontractOperation.h"
#include "smartcontract_executor_operations/ModuleNotFoundException.h"

#include "ext_arguments/NumericArgument.h"

#include "engine/sc_analyze/AnalyzedType.h"
using namespace codablecash;
using namespace alinous;


TEST_GROUP(TestCreateExecOperationGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};

TEST(TestCreateExecOperationGroup, case01){
	const File* projectFolder = this->env->getProjectRoot();
	ModularProjectUtils utils(L"src_test/smartcontract_modular/operation/resources/case01/project01/", projectFolder);
	const File* baseDir = utils.getFolder();

	ModularSmartcontractProject project(baseDir);
	project.loadProject();

	// public key and module address
	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
	ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);
	ScPublicKey publicKey = privateKey->generatePublicKey();

	SmartcontractModuleAddress* moduleAddress = SmartcontractModuleAddress::createAddress(0, &publicKey); __STP(moduleAddress);

	// Executor
	File projectTestFolder = this->env->testCaseDir();
	_ST(File, baseTestDir, projectTestFolder.get(L"exec"))

	ModularSmartcontractExecutor executor(baseTestDir);
	executor.createExecutor();
	{
		executor.open();

		ModularSmartcontractInstance* inst = project.toInstance(); __STP(inst);
		inst->loadCompilantUnits(project.getProjectBaseDir());
		SmartcontractProjectData* data = inst->createData(); __STP(data);
		CHECK(data != nullptr);
		// register
		{
			CdbDatabaseSessionId* registerTrxId = CdbDatabaseSessionId::createRandomId(); __STP(registerTrxId);

			BlockHeaderId* regHeaderId = BlockHeaderId::makeRandomHeaderId(); __STP(regHeaderId);
			uint64_t regHeight = 10;

			RegisterSmartcontractProjectOperation* op = executor.makeRegisterSmartcontractProjectOperation(moduleAddress, data, registerTrxId, regHeight, regHeaderId); __STP(op);
			op->execute(&executor);
		}

		// create instance
		SmartcontractInstanceAddress* address = SmartcontractInstanceAddress::createAddress(0); __STP(address);
		CdbDatabaseSessionId* createTrxId = CdbDatabaseSessionId::createRandomId(); __STP(createTrxId);
		{
			BlockHeaderId* createHeaderId = BlockHeaderId::makeRandomHeaderId(); __STP(createHeaderId);
			uint64_t creHeight = 11;

			SmartcontractProjectId* projectId = project.getProjectId(); __STP(projectId);
			CreateSmartcontractInstanceOperation* op = executor.makeCreateSmartcontractInstanceOperation(address, projectId, createTrxId, creHeight, createHeaderId); __STP(op);
			op->execute(&executor);
		}

		// exec
		CdbDatabaseSessionId* execTrxId = CdbDatabaseSessionId::createRandomId(); __STP(execTrxId);
		{
			BlockHeaderId* createHeaderId = BlockHeaderId::makeRandomHeaderId(); __STP(createHeaderId);
			uint64_t execHeight = 12;

			CdbDatabaseSessionId* lastTrxId = createTrxId;

			ArrayList<AbstractFunctionExtArguments> args;
			args.setDeleteOnExit();
			UnicodeString module(L"exec");
			UnicodeString methodName(L"setCount");

			args.addElement(new NumericArgument(11, AnalyzedType::TYPE_INT));

			ProcessSmartcontractOperation* op = executor.makeProcessSmartcontractOperation(address, execTrxId, lastTrxId, &module, &methodName,&args, execHeight, createHeaderId); __STP(op);
			op->execute(&executor);
		}

		executor.close();
	}
}




