/*
 * test_instance_index.cpp
 *
 *  Created on: Feb 20, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "smartcontract_executor_index/InstanceIdIndex.h"
#include "smartcontract_executor_index/InstanceIdIndexData.h"
#include "smartcontract_executor_index/InstanceIdIndexKey.h"
#include "smartcontract_executor_index/InstanceSessionContext.h"

#include "transaction/SmartcontractInstanceAddress.h"

#include "trx/session/base/CdbDatabaseSessionId.h"

using namespace codablecash;


TEST_GROUP(TestInstanceIndexGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};

TEST(TestInstanceIndexGroup, case01){
	File projectTestFolder = this->env->testCaseDir();
	_ST(File, baseTestDir, projectTestFolder.get(L"index"))

	InstanceIdIndex index(baseTestDir);

	index.createBlankDatabase();

	index.open();
	{
		InstanceIdIndexKey key;
		SmartcontractInstanceAddress* address = SmartcontractInstanceAddress::createAddress(0); __STP(address);
		key.setInstanceAddress(address);
		key.setHeight(1);

		InstanceIdIndexData data;
		{
			InstanceSessionContext* context = new InstanceSessionContext(); __STP(context);
			context->setCallSerial(1);
			CdbDatabaseSessionId* sessionId = CdbDatabaseSessionId::createRandomId(); __STP(sessionId);
			context->setTrxId(sessionId);

			data.addSessionContext(__STP_MV(context));
		}

		index.put(&key, &data);

		InstanceIdIndexData* ret = index.findData(&key); __STP(ret);
		const InstanceSessionContext* ctx = ret->get(0);

		CHECK(ctx->getCallSerial() == 1);
		CHECK(data.get(0)->getSessionId()->equals(ctx->getSessionId()));
	}
	index.close();
}

void addRecord(InstanceIdIndex& index, const SmartcontractInstanceAddress* address, uint64_t height, uint64_t callSerial, const CdbDatabaseSessionId* trxId){
	InstanceIdIndexKey key;
	key.setInstanceAddress(address);
	key.setHeight(height);

	InstanceIdIndexData data;
	{
		InstanceSessionContext* context = new InstanceSessionContext(); __STP(context);
		context->setCallSerial(callSerial);
		context->setTrxId(trxId);

		data.addSessionContext(__STP_MV(context));
	}

	index.put(&key, &data);
}

TEST(TestInstanceIndexGroup, case02){
	File projectTestFolder = this->env->testCaseDir();
	_ST(File, baseTestDir, projectTestFolder.get(L"index"))

	InstanceIdIndex index(baseTestDir);

	index.createBlankDatabase();

	index.open();
	{
		SmartcontractInstanceAddress* address01 = SmartcontractInstanceAddress::createAddress(0); __STP(address01);
		CdbDatabaseSessionId* sessionId01 = CdbDatabaseSessionId::createRandomId(); __STP(sessionId01);
		addRecord(index, address01, 1, 1, sessionId01);

		CdbDatabaseSessionId* sessionId02 = CdbDatabaseSessionId::createRandomId(); __STP(sessionId02);
		addRecord(index, address01, 1, 2, sessionId02);

		InstanceIdIndexKey key;
		key.setInstanceAddress(address01);
		key.setHeight(1);


		{
			InstanceIdIndexData* ret = index.findData(&key); __STP(ret);
			const InstanceSessionContext* ctx = ret->get(0);

			CHECK(ctx->getCallSerial() == 2);
			CHECK(sessionId02->equals(ctx->getSessionId()));
		}
		{
			InstanceIdIndexData* ret = index.findData(&key); __STP(ret);
			const InstanceSessionContext* ctx = ret->get(1);

			CHECK(ctx->getCallSerial() == 1);
			CHECK(sessionId01->equals(ctx->getSessionId()));
		}

	}

	index.close();
}

TEST(TestInstanceIndexGroup, case03){ // remove
	File projectTestFolder = this->env->testCaseDir();
	_ST(File, baseTestDir, projectTestFolder.get(L"index"))

	InstanceIdIndex index(baseTestDir);

	index.createBlankDatabase();
	index.open();
	{
		SmartcontractInstanceAddress* address01 = SmartcontractInstanceAddress::createAddress(0); __STP(address01);
		CdbDatabaseSessionId* sessionId01 = CdbDatabaseSessionId::createRandomId(); __STP(sessionId01);
		addRecord(index, address01, 1, 1, sessionId01);

		CdbDatabaseSessionId* sessionId02 = CdbDatabaseSessionId::createRandomId(); __STP(sessionId02);
		addRecord(index, address01, 1, 2, sessionId02);

		InstanceIdIndexKey key;
		key.setInstanceAddress(address01);
		key.setHeight(1);
		key.setTrxId(sessionId02);

		index.remove(&key);

		key.setTrxId(sessionId01);
		index.remove(&key);
	}

	index.close();
}
