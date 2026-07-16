/*
 * test_sha256.cpp
 *
 *  Created on: 2023/03/10
 *      Author: iizuka
 */

#include "test_utils/t_macros.h"

#include "numeric/BigInteger.h"

#include "ecda/Secp256k1Point.h"

#include "base_io/ByteBuffer.h"

#include "../../src_ext/yescrypt/sha256.h"
using namespace codablecash;

TEST_GROUP(Sha256TestGroup) {
	TEST_SETUP() {}
	TEST_TEARDOWN() {}

};

TEST(Sha256TestGroup, case01){
	BigInteger blockHash = BigInteger::random(BigInteger(0L), BigInteger(Secp256k1Point::p));
	BigInteger nonce = BigInteger::random(BigInteger(0L), BigInteger(Secp256k1Point::p));

	ByteBuffer* bin = blockHash.toBinary(); __STP(bin);
	ByteBuffer* bin2 = nonce.toBinary(); __STP(bin2);

	ByteBuffer* allbuff = ByteBuffer::allocateWithEndian(bin->limit()+bin2->limit(), true);__STP(allbuff);
	allbuff->put(bin);
	allbuff->put(bin2);
	allbuff->position(0);

	uint8_t out1[32]{};
	{
		SHA256_CTX ctx;
		SHA256_Init(&ctx);

		SHA256_Update(&ctx, bin->array(), bin->limit());
		SHA256_Update(&ctx, bin2->array(), bin2->limit());

		SHA256_Final((uint8_t *)out1, &ctx);
	}
	uint8_t out2[32]{};
	{
		SHA256_CTX ctx;
		SHA256_Init(&ctx);

		SHA256_Update(&ctx, allbuff->array(), allbuff->limit());

		SHA256_Final((uint8_t *)out2, &ctx);
	}

	int diff = Mem::memcmp(out1, out2, 32);
	CHECK(diff == 0);
}
