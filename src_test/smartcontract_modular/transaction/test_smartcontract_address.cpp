/*
 * test_smartcontract_address.cpp
 *
 *  Created on: Oct 2, 2025
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "bc_wallet/HdWalletSeed.h"

#include "ecda/ScPrivateKey.h"
#include "ecda/ScPublicKey.h"

#include "transaction/SmartcontractModuleAddress.h"
#include "transaction/SmartcontractInstanceAddress.h"

#include "bc_base/AddressDescriptor.h"

#include "bc_base_utxo_index/AddressDescriptorData.h"


using namespace codablecash;

TEST_GROUP(TestSmartcontractAddressGroup) {
	TEST_SETUP() {
		env->setup();
	}
	TEST_TEARDOWN() {
		env->teardown();
	}
};

TEST(TestSmartcontractAddressGroup, case01) {
	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
	ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);

	ScPublicKey publicKey = privateKey->generatePublicKey();

	SmartcontractModuleAddress* address = SmartcontractModuleAddress::createAddress(1, &publicKey); __STP(address);

	int capacity = address->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);
	address->toBinary(buff);

	buff->position(0);
	AbstractAddress* addr = AbstractAddress::createFromBinary(buff); __STP(addr);

	SmartcontractModuleAddress* address2 = dynamic_cast<SmartcontractModuleAddress*>(addr->copyData()); __STP(address2);

	AddressDescriptor* desc = address->toAddressDescriptor(); __STP(desc);
	AddressDescriptor* desc2 = address2->toAddressDescriptor(); __STP(desc2);

	CHECK(desc->compareTo(desc2) == 0);
}

TEST(TestSmartcontractAddressGroup, case02) {
	HdWalletSeed* seed = HdWalletSeed::newSeed(); __STP(seed);
	BigInteger* bigInt = seed->toBigInteger(); __STP(bigInt);
	ScPrivateKey* privateKey = new ScPrivateKey(bigInt, 1); __STP(privateKey);

	ScPublicKey publicKey = privateKey->generatePublicKey();

	SmartcontractInstanceAddress* address = SmartcontractInstanceAddress::createAddress(1, &publicKey); __STP(address);

	int capacity = address->binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);
	address->toBinary(buff);

	buff->position(0);
	AbstractAddress* addr = AbstractAddress::createFromBinary(buff); __STP(addr);

	SmartcontractInstanceAddress* address2 = dynamic_cast<SmartcontractInstanceAddress*>(addr->copyData()); __STP(address2);

	AddressDescriptor* desc = address->toAddressDescriptor(); __STP(desc);
	AddressDescriptor* desc2 = address2->toAddressDescriptor(); __STP(desc2);

	CHECK(desc->compareTo(desc2) == 0);
}
