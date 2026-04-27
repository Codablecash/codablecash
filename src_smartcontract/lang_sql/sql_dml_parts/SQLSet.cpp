/*
 * SQLSet.cpp
 *
 *  Created on: 2019/02/20
 *      Author: iizuka
 */

#include "lang_sql/sql_dml_parts/SQLSet.h"
#include "lang_sql/sql_dml_parts/SQLSetPair.h"

#include "scan_select/scan_planner/base/UpdateColumnPlanner.h"

namespace alinous {

SQLSet::SQLSet() : AbstractSQLPart(CodeElement::SQL_PART_SET) {
}

SQLSet::~SQLSet() {
	this->list.deleteElements();
}

void SQLSet::addPair(SQLSetPair* pair) noexcept {
	this->list.addElement(pair);
}

void SQLSet::preAnalyze(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLSetPair* pair = this->list.get(i);

		pair->setParent(this);
		pair->preAnalyze(actx);
	}
}

void SQLSet::analyzeTypeRef(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLSetPair* pair = this->list.get(i);

		pair->analyzeTypeRef(actx);
	}
}

void SQLSet::analyze(AnalyzeContext* actx) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLSetPair* pair = this->list.get(i);

		pair->analyze(actx);
	}
}

void SQLSet::init(VirtualMachine* vm) {
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLSetPair* pair = this->list.get(i);

		pair->init(vm);
	}
}

UpdateColumnPlanner* SQLSet::makeUpdateColumnPlanner(VirtualMachine* vm) {
	UpdateColumnPlanner* updatePlanner = new UpdateColumnPlanner();

	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLSetPair* pair = this->list.get(i);

		UpdateColumnPolicy* policy = pair->makeUpdateColumnPolicy(vm);
		updatePlanner->addPolicy(policy);
	}

	return updatePlanner;
}

int SQLSet::binarySize() const {
	int total = sizeof(uint16_t);

	total += sizeof(uint32_t);
	int maxLoop = this->list.size();
	for(int i = 0; i != maxLoop; ++i){
		SQLSetPair* pair = this->list.get(i);
		total += pair->binarySize();
	}

	total += positionBinarySize();

	return total;
}

void SQLSet::toBinary(ByteBuffer* out) const {
	out->putShort(CodeElement::SQL_PART_SET);

	int maxLoop = this->list.size();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		SQLSetPair* pair = this->list.get(i);
		pair->toBinary(out);
	}

	positionToBinary(out);
}

void SQLSet::fromBinary(ByteBuffer* in) {
	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		CodeElement* element = createFromBinary(in);
		checkKind(element, CodeElement::SQL_PART_SET_PAIR);
		SQLSetPair* pair = dynamic_cast<SQLSetPair*>(element);

		this->list.addElement(pair);
	}

	positionFromBinary(in);
}

} /* namespace alinous */
