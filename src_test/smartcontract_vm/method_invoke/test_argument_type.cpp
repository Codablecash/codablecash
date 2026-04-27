/*
 * test_argument_type.cpp
 *
 *  Created on: Mar 12, 2026
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "ext_arguments/ArgumentType.h"

#include "base_io/ByteBuffer.h"

#include "../VmTestUtils.h"

#include "engine/sc/SmartContract.h"

#include "engine/sc_analyze/AnalyzeContext.h"
#include "engine/sc_analyze/PackageSpace.h"
#include "engine/sc_analyze/AnalyzedClass.h"
#include "engine/sc_analyze/AnalyzedType.h"

#include "engine/sc_analyze/TypeResolver.h"

#include "lang/sc_declare/ClassDeclare.h"



TEST_GROUP(TestArgumentTypeGroup) {
	TEST_SETUP(){}
	TEST_TEARDOWN(){}
};

TEST(TestArgumentTypeGroup, case01){
	const File* projectFolder = this->env->getProjectRoot();
	VmTestUtils util(L"src_test/smartcontract_vm/method_invoke/resources/arg/case01/", projectFolder);

	util.loadAllFiles();
	util.setMain(L"test.fw", L"SmartContract", L"main");

	bool result = util.analyze();
	CHECK(result)

	AnalyzeContext* actx = util.sc->getAnalyzeContext();
	TypeResolver* resolver = actx->getTypeResolver();

	UnicodeString package(L"test.fw");
	UnicodeString clazz(L"SmartContract");

	HashMap<UnicodeString, PackageSpace>* spaces = actx->getPackageSpaces();
	PackageSpace* space = spaces->get(&package);

	AnalyzedClass* aclazz = space->getClass(&clazz);
	ClassDeclare* classDec = aclazz->getClassDeclare();

	AnalyzedType* atype = resolver->findClassType(&package, &clazz); __STP(atype);

	ArgumentType argType(atype);

	AnalyzedType* at = argType.toAnalyzedType(resolver, classDec); __STP(at);

	bool bl = at->equals(atype);
	CHECK(bl == true);
}

// genericType
TEST(TestArgumentTypeGroup, case02){
	const File* projectFolder = this->env->getProjectRoot();
	VmTestUtils util(L"src_test/smartcontract_vm/method_invoke/resources/arg/case01/", projectFolder);

	util.loadAllFiles();
	util.setMain(L"test.fw", L"SmartContract", L"main");

	bool result = util.analyze();
	CHECK(result)

	AnalyzeContext* actx = util.sc->getAnalyzeContext();
	TypeResolver* resolver = actx->getTypeResolver();

	UnicodeString package(L"test.fw");
	UnicodeString clazz(L"SmartContract");

	HashMap<UnicodeString, PackageSpace>* spaces = actx->getPackageSpaces();
	PackageSpace* space = spaces->get(&package);

	AnalyzedClass* aclazz = space->getClass(&clazz);
	ClassDeclare* classDec = aclazz->getClassDeclare();

	AnalyzedType* atype = resolver->findClassType(&package, &clazz); __STP(atype);

	UnicodeString genericsType(L"T");
	atype->setGenericsType(&genericsType);

	ArgumentType argType(atype);

	AnalyzedType* at = argType.toAnalyzedType(resolver, classDec); __STP(at);

	bool bl = at->equals(atype);
	CHECK(bl == true);
}

TEST(TestArgumentTypeGroup, binary01){
	const File* projectFolder = this->env->getProjectRoot();
	VmTestUtils util(L"src_test/smartcontract_vm/method_invoke/resources/arg/case01/", projectFolder);

	util.loadAllFiles();
	util.setMain(L"test.fw", L"SmartContract", L"main");

	bool result = util.analyze();
	CHECK(result)

	AnalyzeContext* actx = util.sc->getAnalyzeContext();
	TypeResolver* resolver = actx->getTypeResolver();

	UnicodeString package(L"test.fw");
	UnicodeString clazz(L"SmartContract");

	HashMap<UnicodeString, PackageSpace>* spaces = actx->getPackageSpaces();
	PackageSpace* space = spaces->get(&package);

	AnalyzedClass* aclazz = space->getClass(&clazz);
	ClassDeclare* classDec = aclazz->getClassDeclare();

	AnalyzedType* atype = resolver->findClassType(&package, &clazz); __STP(atype);

	UnicodeString genericsType(L"T");
	atype->setGenericsType(&genericsType);

	ArgumentType argType(atype);

	int cap = argType.binarySize();
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(cap, true); __STP(buff);

	argType.toBinary(buff);
	buff->position(0);

	ArgumentType* argType2 = ArgumentType::createFromBinary(buff);

	int cap2 = argType2->binarySize();
	ByteBuffer* buff2 = ByteBuffer::allocateWithEndian(cap2, true); __STP(buff2);
	argType2->toBinary(buff2);

	int cmp = buff->binaryCmp(buff2);
	CHECK(cmp == 0);
}
