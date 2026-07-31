/*
 * test_transfer_history.cpp
 *
 *  Created on: 2023/09/13
 *      Author: iizuka
 */


#include "test_utils/t_macros.h"

#include "data_history/DataHistoryStore.h"

#include "base/StackRelease.h"

#include "bc_status_cache_extend_shard/NotifyZoneExtendRequestedTransaction.h"

#include "bc_block/BlockHeaderId.h"

#include "bc_block_header_command/BlockHeaderCommandId.h"

#include "data_history_data/TransactionTransferData.h"

#include "data_history/TransferedDataId.h"
using namespace codablecash;


TEST_GROUP(TestTransferHistoryGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestTransferHistoryGroup, case01){
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"codable_home"))

	DataHistoryStore store(baseDir);

	store.createBlankDatabase();

	{
		store.open();

		NotifyZoneExtendRequestedTransaction* trx = new NotifyZoneExtendRequestedTransaction(); __STP(trx);
		{
			BlockHeaderId* headerId = BlockHeaderId::makeRandomHeaderId(); __STP(headerId);
			trx->setHeaderInfo(0, 1, headerId);
			trx->setNewShardZone(1);

			BlockHeaderCommandId* commandId =  BlockHeaderCommandId::makeRandomHeaderId(); __STP(commandId);
			trx->setCommandId(commandId);
			trx->build();
		}

		{
			TransactionTransferData data;
			data.setTransaction(trx);
			TransferedDataId* dataId = data.getTransferedDataId(); __STP(dataId);

			store.add(dataId, &data);

			bool bl = store.hasId(dataId);
			CHECK(bl == true);
		}


		NotifyZoneExtendRequestedTransaction* trx2 = nullptr;
		{
			int cap = trx->binarySize();
			ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
			trx->toBinary(buff);

			buff->position(0);
			AbstractBlockchainTransaction* _t = NotifyZoneExtendRequestedTransaction::createFromBinary(buff);
			trx2 = dynamic_cast<NotifyZoneExtendRequestedTransaction*>(_t);
		}
		__STP(trx2);

		{
			TransactionTransferData data;
			data.setTransaction(trx2);
			TransferedDataId* dataId = data.getTransferedDataId(); __STP(dataId);

			bool bl = store.hasId(dataId);
			CHECK(bl == true);
		}


		store.close();
	}

}


