/*
 * MoveTestUtil.cpp
 *
 *  Created on: Mar 14, 2026
 *      Author: iizuka
 */

#include "MoveTestUtil.h"

#include "modular_project/ModularSmartcontractProject.h"

#include "smartcontract_executor/ModularSmartcontractExecutor.h"

#include "transaction/SmartcontractModuleAddress.h"

#include "bc_wallet/HdWalletSeed.h"

#include "base/StackRelease.h"

#include "base_io/File.h"

#include "ecda/ScPrivateKey.h"
#include "ecda/ScPublicKey.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

#include "bc_block/BlockHeaderId.h"

#include "smartcontract_instance/ModularSmartcontractInstance.h"

#include "modular_project_registory/SmartcontractProjectData.h"

#include "smartcontract_executor_operations/RegisterSmartcontractProjectOperation.h"
#include "smartcontract_executor_operations/ProcessSmartcontractOperation.h"

#include "smartcontract_executor_index/InstanceSessionContext.h"

#include "transaction/SmartcontractInstanceAddress.h"

#include "smartcontract_executor_operations/CreateSmartcontractInstanceOperation.h"

#include <stdio.h>
#include <wchar.h>
#include <cstdio>

#include "modular_project/SmartcontractProjectId.h"

#include "MoveTestSessionHeadDitector.h"


namespace codablecash {

MoveTestUtil::MoveTestUtil(ModularSmartcontractProject* project, File* baseTestDir) {
	this->project = project;
	this->baseTestDir = new File(*baseTestDir);
	this->moduleAddress = nullptr;
	this->executor = nullptr;
}

MoveTestUtil::~MoveTestUtil() {
	if(this->executor != nullptr){
		this->executor->close();
	}

	delete this->baseTestDir;
	delete this->moduleAddress;
	delete this->executor;
}

void MoveTestUtil::init() {
	// public key and module address
	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
	ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);
	ScPublicKey publicKey = privateKey->generatePublicKey();

	this->moduleAddress = SmartcontractModuleAddress::createAddress(0, &publicKey);

	{
		UnicodeString* str = this->moduleAddress->toString(); __STP(str);
		const char* log  = str->toCString();
		::printf("%s\n", log);
		delete [] log;
	}

	// Executor
	this->executor = new ModularSmartcontractExecutor(this->baseTestDir);
	this->executor->createExecutor();

	this->executor->open();
}

void MoveTestUtil::registerProject() {
	BlockHeaderId* regHeaderId = BlockHeaderId::makeRandomHeaderId(); __STP(regHeaderId);
	uint64_t regHeight = 10;

	registerProject(regHeight, regHeaderId);
}

void MoveTestUtil::registerProject(uint64_t regHeight, BlockHeaderId *regHeaderId) {
	ModularSmartcontractInstance* inst = this->project->toInstance(); __STP(inst);
	inst->loadCompilantUnits(project->getProjectBaseDir());
	SmartcontractProjectData* data = inst->createData(); __STP(data);

	CdbDatabaseSessionId* registerTrxId = CdbDatabaseSessionId::createRandomId(); __STP(registerTrxId);

	RegisterSmartcontractProjectOperation* op = this->executor->makeRegisterSmartcontractProjectOperation(this->moduleAddress, data, registerTrxId, regHeight, regHeaderId); __STP(op);
	op->execute(this->executor);
}

void MoveTestUtil::createInstance(const SmartcontractInstanceAddress* instAddress, uint64_t creHeight) {
	BlockHeaderId* blockHeaderId = BlockHeaderId::makeRandomHeaderId(); __STP(blockHeaderId);

	createInstance(instAddress, blockHeaderId, creHeight);
}

void MoveTestUtil::createInstance(const SmartcontractInstanceAddress *instAddress, BlockHeaderId *blockHeaderId, uint64_t height) {
	SmartcontractProjectId* projectId = this->executor->getLatestProjectId(this->moduleAddress); __STP(projectId);

	CdbDatabaseSessionId* createTrxId = CdbDatabaseSessionId::createRandomId(); __STP(createTrxId);

	CreateSmartcontractInstanceOperation* op
		= this->executor->makeCreateSmartcontractInstanceOperation(instAddress, projectId, createTrxId, height, blockHeaderId); __STP(op);
	op->execute(this->executor);
}

void MoveTestUtil::process(const SmartcontractInstanceAddress* address, const UnicodeString* module, const UnicodeString* methodName, ArrayList<AbstractFunctionExtArguments>* args
		, BlockHeaderId *blockHeaderId, uint64_t height) {
	CdbDatabaseSessionId* execTrxId = CdbDatabaseSessionId::createRandomId(); __STP(execTrxId);

	SmartcontractExecContextRegistory* contextRegistory = this->executor->getSmartcontractExecContextRegistory();
	MoveTestSessionHeadDitector detector(contextRegistory);

	this->executor->findHeads(address, &detector);

	ArrayList<InstanceSessionContext>* list = detector.getHeads();
	uint64_t callSerial = 0;

	CdbDatabaseSessionId* lastTrxId = nullptr;

	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		InstanceSessionContext* ctx = list->get(i);

		uint64_t sel = ctx->getCallSerial();
		if(callSerial < sel){
			delete lastTrxId;
			const CdbDatabaseSessionId* trxId =  ctx->getSessionId();
			lastTrxId = dynamic_cast<CdbDatabaseSessionId*>(trxId->copyData());

			callSerial = sel;
		}
	}

	__STP(lastTrxId);

	ProcessSmartcontractOperation* op = this->executor->makeProcessSmartcontractOperation(address, execTrxId, lastTrxId, module, methodName,args, height, blockHeaderId); __STP(op);
	op->execute(this->executor);

}

CdbDatabaseSessionId* MoveTestUtil::process(const SmartcontractInstanceAddress *address, const UnicodeString *module, const UnicodeString *methodName, ArrayList<AbstractFunctionExtArguments> *args,
		BlockHeaderId *blockHeaderId, uint64_t height, const CdbDatabaseSessionId *lastTrxId) {
	CdbDatabaseSessionId* execTrxId = CdbDatabaseSessionId::createRandomId(); __STP(execTrxId);

	ProcessSmartcontractOperation* op = this->executor->makeProcessSmartcontractOperation(address, execTrxId, lastTrxId, module, methodName, args, height, blockHeaderId); __STP(op);
	op->execute(this->executor);

	const CdbDatabaseSessionId* sessionId = op->getTrxId();

	return dynamic_cast<CdbDatabaseSessionId*>(sessionId->copyData());
}

ArrayList<InstanceSessionContext>* MoveTestUtil::getHeads(const SmartcontractInstanceAddress* address) const {
	SmartcontractExecContextRegistory* contextRegistory = this->executor->getSmartcontractExecContextRegistory();
	MoveTestSessionHeadDitector detector(contextRegistory);

	this->executor->findHeads(address, &detector);

	ArrayList<InstanceSessionContext>* ret = new ArrayList<InstanceSessionContext>(); __STP(ret);

	ArrayList<InstanceSessionContext>* list = detector.getHeads();
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		const InstanceSessionContext* ctx = list->get(i);
		ret->addElement(dynamic_cast<InstanceSessionContext*>(ctx->copyData()));
	}

	return __STP_MV(ret);
}



} /* namespace codablecash */
