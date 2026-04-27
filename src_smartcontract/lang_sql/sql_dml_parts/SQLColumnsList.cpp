/*
 * SQLColumnsList.cpp
 *
 *  Created on: 2019/02/16
 *      Author: iizuka
 */

#include "lang_sql/sql_dml_parts/SQLColumnsList.h"
#include "lang_sql/sql_expression/SQLColumnIdentifier.h"

namespace alinous {

SQLColumnsList::SQLColumnsList() : AbstractSQLPart(CodeElement::SQL_PART_COLUMN_LIST) {
}

SQLColumnsList::~SQLColumnsList() {
	this->list.deleteElements();
}

void SQLColumnsList::addColumn(SQLColumnIdentifier* colId) noexcept {
	this->list.addElement(colId);
}

void SQLColumnsList::preAnalyze(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLColumnIdentifier* colId = this->list.get(i);

		colId->setParent(this);
		colId->preAnalyze(actx);
	}
}

void SQLColumnsList::analyzeTypeRef(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLColumnIdentifier* colId = this->list.get(i);

		colId->analyzeTypeRef(actx);
	}
}

void SQLColumnsList::analyze(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLColumnIdentifier* colId = this->list.get(i);

		colId->analyze(actx);
	}
}

void SQLColumnsList::init(VirtualMachine* vm) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLColumnIdentifier* colId = this->list.get(i);

		colId->init(vm);
	}
}

int SQLColumnsList::binarySize() const {
	int total = sizeof(uint16_t);

	total += sizeof(uint32_t);
	int maxLoop = this->list.size();

	for(int i = 0; i != maxLoop; ++i){
		SQLColumnIdentifier* colId = this->list.get(i);
		total += colId->binarySize();
	}

	total += positionBinarySize();

	return total;
}

int SQLColumnsList::numColumns() const noexcept {
	return this->list.size();
}

void SQLColumnsList::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::SQL_PART_COLUMN_LIST);

	int maxLoop = this->list.size();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		SQLColumnIdentifier* colId = this->list.get(i);
		colId->toBinary(out);
	}

	positionToBinary(out);
}

void SQLColumnsList::fromBinary(ByteBuffer* in) {
	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = createFromBinary(in);
		checkKind(element, CodeElement::SQL_EXP_COLUMN_ID);

		SQLColumnIdentifier* colId = dynamic_cast<SQLColumnIdentifier*>(element);
		this->list.addElement(colId);
	}

	positionFromBinary(in);
}

} /* namespace alinous */
