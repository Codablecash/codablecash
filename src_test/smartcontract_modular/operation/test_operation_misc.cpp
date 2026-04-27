/*
 * test_operation_misc.cpp
 *
 *  Created on: Jan 29, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "base_io/ByteBuffer.h"
#include "smartcontract_executor_operations_data/AbstractSmartcontractOperationData.h"

#include "filestore_block/exceptions.h"

using namespace codablecash;
using namespace alinous;

TEST_GROUP(TestOperationMiscGroup) {
	TEST_SETUP() {
	}
	TEST_TEARDOWN() {
	}
};


TEST(TestOperationMiscGroup, case_err01){
	int cap = 10;

	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);
	buff->put(101);
	buff->position(0);

	BinaryFormatException* ex = nullptr;
	try{
		AbstractSmartcontractOperationData::createFromBinary(buff);
	}
	catch(BinaryFormatException* e){
		ex = e;
	}

	CHECK(ex != nullptr);
	delete ex;
}
