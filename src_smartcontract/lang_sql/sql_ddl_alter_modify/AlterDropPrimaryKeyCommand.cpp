/*
 * AlterDropPrimaryKeyCommand.cpp
 *
 *  Created on: 2020/09/09
 *      Author: iizuka
 */

#include "lang_sql/sql_ddl_alter_modify/AlterDropPrimaryKeyCommand.h"

#include "trx/transaction_log_alter_modify/AlterDropPrimaryKeyCommandLog.h"

#include "engine/CodableDatabase.h"
#include "engine/CdbException.h"

#include "schema_table/schema/SchemaManager.h"

#include "vm/VirtualMachine.h"

#include "lang_sql/sql_join_parts/TableIdentifier.h"

#include "schema_table/table/CdbTable.h"


namespace alinous {

AlterDropPrimaryKeyCommand::AlterDropPrimaryKeyCommand(const AlterDropPrimaryKeyCommand& inst)
	: AbstractAlterDdlCommand(CodeElement::DDL_ALTER_DROP_PRIMARY_KEY) {
}

AlterDropPrimaryKeyCommand::AlterDropPrimaryKeyCommand() : AbstractAlterDdlCommand(CodeElement::DDL_ALTER_DROP_PRIMARY_KEY) {

}

AlterDropPrimaryKeyCommand::~AlterDropPrimaryKeyCommand() {

}

int AlterDropPrimaryKeyCommand::binarySize() const {
	int total = sizeof(uint16_t);

	total += positionBinarySize();

	return total;
}

void AlterDropPrimaryKeyCommand::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::DDL_ALTER_DROP_PRIMARY_KEY);

	positionToBinary(out);
}

void AlterDropPrimaryKeyCommand::fromBinary(ByteBuffer* in) {
	positionFromBinary(in);
}

AbstractAlterCommandLog* AlterDropPrimaryKeyCommand::getCommandLog(VirtualMachine* vm) {
	CodableDatabase* db = vm->getDb();
	SchemaManager* scManager = db->getSchemaManager();
	uint64_t commitId = scManager->newCommitId();

	AlterDropPrimaryKeyCommandLog* log = new AlterDropPrimaryKeyCommandLog(commitId);
	log->setCommand(new AlterDropPrimaryKeyCommand(*this));

	return log;
}

void AlterDropPrimaryKeyCommand::preAnalyze(AnalyzeContext* actx) {
}

void AlterDropPrimaryKeyCommand::analyzeTypeRef(AnalyzeContext* actx) {
}

void AlterDropPrimaryKeyCommand::analyze(AnalyzeContext* actx) {
}

void AlterDropPrimaryKeyCommand::interpret(VirtualMachine* vm, AbstractAlterCommandLog* log, TableIdentifier* tableId) {
	AlterDropPrimaryKeyCommandLog* dropPrimaryKeyLog = dynamic_cast<AlterDropPrimaryKeyCommandLog*>(log);
	const AlterDropPrimaryKeyCommand* command = dropPrimaryKeyLog->getCommand();

	CodableDatabase* db = vm->getDb();
	SchemaManager* schemaManager = db->getSchemaManager();

	const UnicodeString* defaultSchema = vm->getCurrentSchema();
	tableId->inputDefaultSchema(defaultSchema);

	CdbTable* table = schemaManager->getTable(tableId, nullptr); // throws if Table does not exists;

	CdbTableIndex* pkey = table->getPrimaryKey();
	if(pkey == nullptr){
		throw new CdbException(L"Primary key does not exists", __FILE__, __LINE__);
	}
}



} /* namespace alinous */
