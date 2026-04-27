/*
 * test_musig.cpp
 *
 *  Created on: 2023/02/01
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "musig/MuSigBuilder.h"
#include "musig/SimpleMuSigSigner.h"
#include "musig/MuSig.h"

#include "ecda/ScPrivateKey.h"

#include "musig/IMuSigSigner.h"
using namespace codablecash;

TEST_GROUP(MuSigTestGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};

TEST(MuSigTestGroup, case01){
	ScPrivateKey key01, key02;

	MuSigBuilder builder;
	builder.addSigner(new SimpleMuSigSigner(key01.getKeyvalue()));
	builder.addSigner(new SimpleMuSigSigner(key02.getKeyvalue()));

	const char* data = "dasdasflgjk;lfdjgk;ldfgjioviypiyvoucsormi;umguarce,@gaopcpopcxrufnkxeyfyen";
	int length = ::strlen(data);

	MuSig sig = builder.sign(data, length);
	{
		/**
		 * 	"R : 1a21a6225240c09d8e9406a53e280a35eff647e45576fecf6281691c0e930129,
		 * 	fd6663a0e4016dad1b2a14ed7eef644c154c3acef437d8ebc49011cb1d3261b9"
		 *
		 */
		UnicodeString* str = sig.toString(); __STP(str);
	}

	ArrayList<IMuSigSigner>* list = builder.getSigners();
	int maxLoop = list->size();
	for(int i = 0; i != maxLoop; ++i){
		IMuSigSigner* signer = list->get(i);
		Secp256k1Point Xi = signer->getxG();

		Secp256k1CompressedPoint point = Secp256k1CompressedPoint::compress(&Xi);
		Secp256k1Point Xi2 = point.decompress();

		bool bl = Xi.equals(Xi2);
		CHECK(bl);

		sig.addXi(Xi2);
	}

	bool bl = sig.verify(data, length);
	CHECK(bl);

	MuSig sig2(sig);
	sig2 = sig;

//	clock_t begin, end;
//	begin = clock();

	bl = sig.verify(data, length);

//	end = clock();
//	printf("Execution Time = %lf [s]\n", (double)(end - begin) / CLOCKS_PER_SEC);

	CHECK(bl);
}
