/*
 * test_scan_const_params.cpp
 *
 *  Created on: 2021/11/15
 *      Author: iizuka
 */
#include "test_utils/t_macros.h"

#include "base/StackRelease.h"

#include "../../toolkit/TestDbSchema02.h"
#include "engine/compiler/SmartContractParser.h"

#include "alinous_lang/AlinousLang.h"

#include "ext_binary/AbstractExtObject.h"
#include "ext_binary/ExtDomArrayObject.h"
#include "ext_binary/ExtDomObject.h"
#include "ext_binary/ExtPrimitiveObject.h"
#include "ext_binary/ExtStringClass.h"

#include "scan_select/scan_columns/scan_columns_params/NullLiteralColumnParam.h"
using namespace codablecash;

TEST_GROUP(TestScanConstParamsGroup) {
	TEST_SETUP(){
		env->setup();
	}
	TEST_TEARDOWN(){
		env->teardown();
	}
};

TEST(TestScanConstParamsGroup, case01){
	const File* projectFolder = this->env->getProjectRoot();
	_ST(File, sourceFile, projectFolder->get(L"src_test/smartcontract_db/select/scan_select_scan_columns/resources/const/case01/select01.alns"))

	TestDbSchema02 schem(this->env);
	schem.init(1024*512);
	{
		SmartContractParser parser(sourceFile);
		AlinousLang* lang = parser.getDebugAlinousLang();

		SelectStatement* stmt = lang->selectStatement(); __STP(stmt);
		CHECK(!parser.hasError())

		schem.execSelectStmt(stmt);

		const AbstractExtObject* exobj = schem.getExtResultObj();
		const ExtDomArrayObject* array = dynamic_cast<const ExtDomArrayObject*>(exobj);

		int size = array->size();
		CHECK(size == 1);

		exobj = array->get(0);
		CHECK(exobj != nullptr);

		const ExtDomObject* domObj = dynamic_cast<const ExtDomObject*>(exobj);
		AbstractExtObject* col = domObj->get(L"_0");
		CHECK(col != nullptr);

		const ExtPrimitiveObject* pr = dynamic_cast<const ExtPrimitiveObject*>(col);
		CHECK(pr != nullptr);

		int32_t n = pr->getIntValue();
		CHECK(n == 1);
	}
}

/**
SELECT 1 AS col0 INTO res
FROM
	test_table;
 */
TEST(TestScanConstParamsGroup, case02){
	const File* projectFolder = this->env->getProjectRoot();
	_ST(File, sourceFile, projectFolder->get(L"src_test/smartcontract_db/select/scan_select_scan_columns/resources/const/case02/select01.alns"))

	TestDbSchema02 schem(this->env);
	schem.init(1024*512);
	{
		SmartContractParser parser(sourceFile);
		AlinousLang* lang = parser.getDebugAlinousLang();

		SelectStatement* stmt = lang->selectStatement(); __STP(stmt);
		CHECK(!parser.hasError())

		schem.execSelectStmt(stmt);

		const AbstractExtObject* exobj = schem.getExtResultObj();
		const ExtDomArrayObject* array = dynamic_cast<const ExtDomArrayObject*>(exobj);

		int size = array->size();
		CHECK(size == 1);

		exobj = array->get(0);
		CHECK(exobj != nullptr);

		const ExtDomObject* domObj = dynamic_cast<const ExtDomObject*>(exobj);
		AbstractExtObject* col = domObj->get(L"col0");
		CHECK(col != nullptr);

		const ExtPrimitiveObject* pr = dynamic_cast<const ExtPrimitiveObject*>(col);
		CHECK(pr != nullptr);

		int32_t n = pr->getIntValue();
		CHECK(n == 1);
	}
}

TEST(TestScanConstParamsGroup, casebool01){
	const File* projectFolder = this->env->getProjectRoot();
	_ST(File, sourceFile, projectFolder->get(L"src_test/smartcontract_db/select/scan_select_scan_columns/resources/const/casebool01/select01.alns"))

	TestDbSchema02 schem(this->env);
	schem.init(1024*512);
	{
		SmartContractParser parser(sourceFile);
		AlinousLang* lang = parser.getDebugAlinousLang();

		SelectStatement* stmt = lang->selectStatement(); __STP(stmt);
		CHECK(!parser.hasError())

		schem.execSelectStmt(stmt);

		const AbstractExtObject* exobj = schem.getExtResultObj();
		const ExtDomArrayObject* array = dynamic_cast<const ExtDomArrayObject*>(exobj);

		int size = array->size();
		CHECK(size == 1);

		exobj = array->get(0);
		CHECK(exobj != nullptr);

		const ExtDomObject* domObj = dynamic_cast<const ExtDomObject*>(exobj);
		AbstractExtObject* col = domObj->get(L"col0");
		CHECK(col != nullptr);

		const ExtPrimitiveObject* pr = dynamic_cast<const ExtPrimitiveObject*>(col);
		CHECK(pr != nullptr);

		CHECK(pr->getBoolValue() == true)
	}
}

TEST(TestScanConstParamsGroup, casestr01){
	const File* projectFolder = this->env->getProjectRoot();
	_ST(File, sourceFile, projectFolder->get(L"src_test/smartcontract_db/select/scan_select_scan_columns/resources/const/casestr01/select01.alns"))

	TestDbSchema02 schem(this->env);
	schem.init(1024*512);
	{
		SmartContractParser parser(sourceFile);
		AlinousLang* lang = parser.getDebugAlinousLang();

		SelectStatement* stmt = lang->selectStatement(); __STP(stmt);
		CHECK(!parser.hasError())

		schem.execSelectStmt(stmt);

		const AbstractExtObject* exobj = schem.getExtResultObj();
		const ExtDomArrayObject* array = dynamic_cast<const ExtDomArrayObject*>(exobj);

		int size = array->size();
		CHECK(size == 1);

		exobj = array->get(0);
		CHECK(exobj != nullptr);

		const ExtDomObject* domObj = dynamic_cast<const ExtDomObject*>(exobj);
		AbstractExtObject* col = domObj->get(L"col0");
		CHECK(col != nullptr);

		const ExtStringClass* exstr = dynamic_cast<const ExtStringClass*>(col);
		CHECK(exstr != nullptr);
	}
}

TEST(TestScanConstParamsGroup, casenull01){
	const File* projectFolder = this->env->getProjectRoot();
	_ST(File, sourceFile, projectFolder->get(L"src_test/smartcontract_db/select/scan_select_scan_columns/resources/const/casenull01/select01.alns"))

	TestDbSchema02 schem(this->env);
	schem.init(1024*512);
	{
		SmartContractParser parser(sourceFile);
		AlinousLang* lang = parser.getDebugAlinousLang();

		SelectStatement* stmt = lang->selectStatement(); __STP(stmt);
		CHECK(!parser.hasError())

		schem.execSelectStmt(stmt);

		const AbstractExtObject* exobj = schem.getExtResultObj();
		const ExtDomArrayObject* array = dynamic_cast<const ExtDomArrayObject*>(exobj);

		int size = array->size();
		CHECK(size == 1);

		exobj = array->get(0);
		CHECK(exobj != nullptr);

		const ExtDomObject* domObj = dynamic_cast<const ExtDomObject*>(exobj);
		AbstractExtObject* col = domObj->get(L"col0");

		CHECK(col->isNull());
	}
}

TEST(TestScanConstParamsGroup, casenull02){
	NullLiteralColumnParam param;

	const UnicodeString* str = param.toStringCode();
	str->equals(L"null");
}

