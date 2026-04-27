/*
 * test_reverse_scan.cpp
 *
 *  Created on: 2023/02/28
 *      Author: iizuka
 */

#include "random_access_file/DiskCacheManager.h"

#include "btreekey/BtreeKeyFactory.h"

#include "btree/Btree.h"

#include "btree/BtreeConfig.h"

#include "btree/BtreeScanner.h"

#include "btreekey/ULongKey.h"

#include "btree/BtreeReverseScanner.h"
#include "test_utils/t_macros.h"
#include "TempValue.h"

TEST_GROUP(TestBTreeReverseScanGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

static void addKeyValue(uint64_t key, uint64_t value, Btree* btree){
	ULongKey lkey(key);
	TempValue tvalue(value);

	btree->putData(&lkey, &tvalue);
}

static void setUpdate(Btree& btree, RawArrayPrimitive<uint64_t> &answers){

	{
		addKeyValue(10, 10, &btree);
		addKeyValue(6, 6, &btree);
		addKeyValue(6, 6, &btree);

		addKeyValue(3, 3, &btree);
		addKeyValue(2, 2, &btree);
		addKeyValue(100, 100, &btree);
		addKeyValue(50, 50, &btree);
		addKeyValue(7, 7, &btree);
		addKeyValue(8, 8, &btree);
		addKeyValue(9, 9, &btree);
		addKeyValue(11, 11, &btree);
		addKeyValue(12, 12, &btree);
		addKeyValue(13, 13, &btree);
		addKeyValue(14, 14, &btree);


		answers.addElement(2);
		answers.addElement(3);
		answers.addElement(6);
		answers.addElement(7);
		answers.addElement(8);
		answers.addElement(9); // 5
		answers.addElement(10);
		answers.addElement(11);
		answers.addElement(12);
		answers.addElement(13);
		answers.addElement(14); // 10
		answers.addElement(50);
		answers.addElement(100);
	}

}


TEST(TestBTreeReverseScanGroup, case01){
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"store"))
	_ST(UnicodeString, baseDirStr, baseDir->getAbsolutePath())

	DiskCacheManager cacheManager;
	UnicodeString name(L"file01");
	BtreeKeyFactory* factory = new BtreeKeyFactory(); __STP(factory);
	TmpValueFactory* dfactory = new TmpValueFactory(); __STP(dfactory);

	Btree btree(baseDir, &name, &cacheManager, factory, dfactory);

	BtreeConfig config;
	config.nodeNumber  = 3;
	btree.create(&config);

	BtreeOpenConfig opconf;
	btree.open(&opconf);

	RawArrayPrimitive<uint64_t> answers(32);
	setUpdate(btree, answers);

	{
		BtreeReverseScanner* scanner = btree.getReverseScanner();
		StackRelease<BtreeReverseScanner> __st_scanner(scanner);

		scanner->begin();

		int i = answers.size() - 1;
		while(scanner->hasPrevious()){
			const AbstractBtreeKey* k = scanner->previousKey();
			const IBlockObject* obj = scanner->previous();


			const TempValue* tmp = dynamic_cast<const TempValue*>(obj);
			uint64_t v = tmp->getValue();

			const ULongKey* lk = dynamic_cast<const ULongKey*>(k);
			uint64_t kv = lk->getValue();

			uint64_t a = answers.get(i--);
			CHECK(v == a)
			CHECK(kv == a)

			if(i == 9){
				i = 9;
			}
		}
		CHECK(i == -1);
	}

	btree.close();
}

TEST(TestBTreeReverseScanGroup, case02){
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"store"))
	_ST(UnicodeString, baseDirStr, baseDir->getAbsolutePath())

	DiskCacheManager cacheManager;
	UnicodeString name(L"file01");
	BtreeKeyFactory* factory = new BtreeKeyFactory(); __STP(factory);
	TmpValueFactory* dfactory = new TmpValueFactory(); __STP(dfactory);

	Btree btree(baseDir, &name, &cacheManager, factory, dfactory);

	BtreeConfig config;
	config.nodeNumber  = 3;
	btree.create(&config);

	BtreeOpenConfig opconf;
	btree.open(&opconf);

	RawArrayPrimitive<uint64_t> answers(32);
	setUpdate(btree, answers);

	{
		ULongKey lkey(80);

		BtreeReverseScanner* scanner = btree.getReverseScanner();
		StackRelease<BtreeReverseScanner> __st_scanner(scanner);

		scanner->begin(&lkey);

		int i = answers.size() - 2;
		while(scanner->hasPrevious()){
			const AbstractBtreeKey* k = scanner->previousKey();
			const IBlockObject* obj = scanner->previous();


			const TempValue* tmp = dynamic_cast<const TempValue*>(obj);
			uint64_t v = tmp->getValue();

			const ULongKey* lk = dynamic_cast<const ULongKey*>(k);
			uint64_t kv = lk->getValue();

			uint64_t a = answers.get(i--);
			CHECK(v == a)
			CHECK(kv == a)
		}
		CHECK(i == -1);
	}

	{
		ULongKey lkey(10);
		BtreeReverseScanner* scanner = btree.getReverseScanner();
		StackRelease<BtreeReverseScanner> __st_scanner(scanner);

		scanner->begin(&lkey);
		int i = 6;
		while(scanner->hasPrevious()){
			const AbstractBtreeKey* k = scanner->previousKey();
			const IBlockObject* obj = scanner->previous();


			const TempValue* tmp = dynamic_cast<const TempValue*>(obj);
			uint64_t v = tmp->getValue();

			const ULongKey* lk = dynamic_cast<const ULongKey*>(k);
			uint64_t kv = lk->getValue();

			uint64_t a = answers.get(i--);
			CHECK(v == a)
			CHECK(kv == a)
		}
		CHECK(i == -1);
	}

	{
		ULongKey lkey(1);

		BtreeReverseScanner* scanner = btree.getReverseScanner();
		StackRelease<BtreeReverseScanner> __st_scanner(scanner);

		scanner->begin(&lkey);
		bool bl = scanner->hasPrevious();
		CHECK(bl == false);
	}

	btree.close();
}

// blank
TEST(TestBTreeReverseScanGroup, case03){
	File projectFolder = this->env->testCaseDir();
	_ST(File, baseDir, projectFolder.get(L"store"))
	_ST(UnicodeString, baseDirStr, baseDir->getAbsolutePath())

	DiskCacheManager cacheManager;
	UnicodeString name(L"file01");
	BtreeKeyFactory* factory = new BtreeKeyFactory(); __STP(factory);
	TmpValueFactory* dfactory = new TmpValueFactory(); __STP(dfactory);

	Btree btree(baseDir, &name, &cacheManager, factory, dfactory);

	BtreeConfig config;
	config.nodeNumber  = 3;
	btree.create(&config);

	BtreeOpenConfig opconf;
	btree.open(&opconf);

	RawArrayPrimitive<uint64_t> answers(32);

	{
		BtreeReverseScanner* scanner = btree.getReverseScanner();
		StackRelease<BtreeReverseScanner> __st_scanner(scanner);

		scanner->begin();
		bool bl = scanner->hasPrevious();
		CHECK(bl == false);

		bl = scanner->hasPrevious();
		CHECK(bl == false);
	}

	setUpdate(btree, answers);

	btree.close();
}

