/*
 * TestDbSchema0Index01.cpp
 *
 *  Created on: 2023/06/23
 *      Author: iizuka
 */

#include "TestDbSchema0Index01.h"

#include "trx/transaction/CdbTransaction.h"

#include "trx/transaction_log/CreateTableLog.h"

#include "engine/CodableDatabase.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "schema_table/table/CdbTable.h"

#include "trx/transaction_log/InsertLog.h"

#include "schema_table/record/table_record/CdbTableIdentifier.h"
#include "schema_table/record/table_record/CdbRecord.h"

#include "schema_table/record/table_record_local/LocalCdbOid.h"

#include "schema_table/record/table_record_value/CdbIntValue.h"
#include "schema_table/record/table_record_value/CdbStringValue.h"
#include "schema_table/record/table_record_value/AbstractCdbValue.h"

namespace codablecash {

TestDbSchema0Index01::TestDbSchema0Index01(TestEnv* env) : TestDbSchemaBase(env) {

}

TestDbSchema0Index01::~TestDbSchema0Index01() {

}

void TestDbSchema0Index01::init(uint64_t memCapacity) {
	TestDbSchemaBase::init(memCapacity);
	createTable();
}

void TestDbSchema0Index01::createTable() {
	CodableDatabase* db = getDatabase();
	CdbTransaction* trx = db->newTransaction(); __STP(trx);

	CreateTableLog* cmd = new CreateTableLog(1L);
	CdbTable* table = new CdbTable(0);

	UnicodeString* testSchema = new UnicodeString(L"public");
	table->setSchemaName(testSchema);
	table->setName(new UnicodeString(L"test_table"));

	table->addColumn(10, L"id", AbstractCdbValue::TYPE_INT, 0, false, false, nullptr);
	table->addColumn(11, L"name", AbstractCdbValue::TYPE_STRING, 8, false, false, L"");
	table->addColumn(12, L"email_id", AbstractCdbValue::TYPE_INT, 0, false, false, L"0");
	table->addColumn(13, L"comment", AbstractCdbValue::TYPE_STRING, 0, false, false, L"");

	table->setPrimaryKey(L"id");

	cmd->setTable(table);

	trx->createTable(cmd);
	trx->commit();
}

void TestDbSchema0Index01::insert01() {
	CdbTransaction* trx = getDatabase()->newTransaction(); __STP(trx);

	insertRecord(trx, 1, L"tanaka", 11, L"test1");
	insertRecord(trx, 2, L"sato", 12, L"test2");
	insertRecord(trx, 3, L"saito", 12, L"test3");

	trx->commit();
}

void TestDbSchema0Index01::insertRecord(CdbTransaction *trx, int id, const wchar_t *name, int email_id, const wchar_t *comment) {
	InsertLog* log = new InsertLog(1L);

	CdbTableIdentifier* tableId = new CdbTableIdentifier();
	tableId->setTable(new UnicodeString(L"test_table"));
	log->setTable(tableId);

	CdbRecord* record = new CdbRecord();
	LocalCdbOid loid(this->loidSerial++);
	record->setOid(&loid);

	record->addValue(new CdbIntValue(id));

	if(name != nullptr){
		record->addValue(new CdbStringValue(name));
	}
	else{
		record->addValue(nullptr);
	}

	record->addValue(new CdbIntValue(email_id));
	record->addValue(new CdbStringValue(comment));

	log->addRecord(record);

	trx->insert(log);
}

} /* namespace codablecash */
