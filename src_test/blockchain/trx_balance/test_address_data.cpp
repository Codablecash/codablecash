/*
 * test_address_data.cpp
 *
 *  Created on: 2023/02/26
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "bc_wallet/HdWalletSeed.h"

#include "bc_base/BalanceAddress.h"
#include "bc_base/AddressDescriptor.h"

#include "ecda/ScPrivateKey.h"
#include "ecda/ScPublicKey.h"

#include "base_io/ByteBuffer.h"

#include "bc_base_utxo_index/AddressDescriptorData.h"

using namespace codablecash;
using namespace alinous;

TEST_GROUP(TestAddressDataGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};

TEST(TestAddressDataGroup, case01){
	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
	ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);

	ScPublicKey publicKey = privateKey->generatePublicKey();

	BalanceAddress* address = BalanceAddress::createAddress(1, &publicKey); __STP(address);

	int capacity = address->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);
	address->toBinary(buff);

	buff->position(0);
	AbstractAddress* addr = BalanceAddress::createFromBinary(buff); __STP(addr);

	BalanceAddress* address2 = dynamic_cast<BalanceAddress*>(addr->copyData()); __STP(address2);

	AddressDescriptor* desc = address->toAddressDescriptor(); __STP(desc);
	AddressDescriptor* desc2 = address2->toAddressDescriptor(); __STP(desc2);

	CHECK(desc->compareTo(desc2) == 0);

	AddressDescriptorData* data = new AddressDescriptorData(desc); __STP(data);
	AddressDescriptorData* data2 = dynamic_cast<AddressDescriptorData*>(data->copyData()); __STP(data2);

	CHECK(desc->compareTo(data2->getDescriptor()) == 0);
}

TEST(TestAddressDataGroup, case02){
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(10, true); __STP(buff);
	AbstractAddress* addr = BalanceAddress::createFromBinary(buff); __STP(addr);

	CHECK(addr == nullptr);
}

