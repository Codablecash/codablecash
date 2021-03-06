/*
 * CdbRecordKey.cpp
 *
 *  Created on: 2020/05/30
 *      Author: iizuka
 */

#include "schema_table/record/table_record_key/CdbRecordKey.h"

#include "btree/AbstractBtreeKey.h"

#include "base_io/ByteBuffer.h"

#include "schema_table/record/table_record/CdbKeyFactory.h"

namespace codablecash {

CdbRecordKey::CdbRecordKey(const CdbRecordKey& inst) : AbstractCdbKey(AbstractCdbKey::TYPE_RECORD_KEY) {
	int maxLoop = inst.list.size();
	for(int i = 0; i != maxLoop; ++i){
		AbstractBtreeKey* key = inst.list.get(i);

		if(key == nullptr){
			this->list.addElement(nullptr);
			continue;
		}

		key = dynamic_cast<AbstractBtreeKey*>(key->clone());
		this->list.addElement(key);
	}
}

CdbRecordKey::CdbRecordKey() : AbstractCdbKey(AbstractCdbKey::TYPE_RECORD_KEY) {

}

CdbRecordKey::~CdbRecordKey() {
	this->list.deleteElements();
}

AbstractBtreeKey* CdbRecordKey::clone() const noexcept {
	return new CdbRecordKey(*this);
}

int CdbRecordKey::compareTo(const AbstractBtreeKey* key) const noexcept {
	if(key->isInfinity()){
		return -1;
	}
	else if(key->isNull()){
		return 1;
	}

	const CdbRecordKey* right = dynamic_cast<const CdbRecordKey*>(key);

	int result = 0;

	int leftSize = this->list.size();
	for(int i = 0; i != leftSize; ++i){
		AbstractBtreeKey* l = this->list.get(i);
		AbstractBtreeKey* r = right->list.get(i);

		result = l != nullptr ? l->compareTo(r) : l - r;

		if(result == 0){
			continue;
		}

		break;
	}

	return result;
}

int CdbRecordKey::binarySize() const {
	int total = sizeof(uint32_t);

	int maxLoop = this->list.size();
	total += sizeof(int32_t);

	for(int i = 0; i != maxLoop; ++i){
		AbstractBtreeKey* key = this->list.get(i);

		if(key == nullptr){
			total += sizeof(uint32_t);
			continue;
		}

		total += key->binarySize();
	}

	return total;
}

void CdbRecordKey::toBinary(ByteBuffer* out) const {
	out->putInt(this->type);

	int maxLoop = this->list.size();
	out->putInt(maxLoop);

	for(int i = 0; i != maxLoop; ++i){
		AbstractBtreeKey* key = this->list.get(i);

		if(key == nullptr){
			out->putInt(0);
			continue;
		}

		key->toBinary(out);
	}
}

void CdbRecordKey::fromBinary(ByteBuffer* in) {
	CdbKeyFactory factory;

	int maxLoop = in->getInt();
	for(int i = 0; i != maxLoop; ++i){
		uint32_t keyType = in->getInt();
		AbstractBtreeKey* abkey = factory.fromBinary(keyType, in);

		AbstractBtreeKey* key = dynamic_cast<AbstractBtreeKey*>(abkey);
		//assert(key != nullptr);

		addKey(key);
	}

}

void CdbRecordKey::addKey(AbstractBtreeKey* key) noexcept {
	//assert(key != nullptr);

	this->list.addElement(key);
}

} /* namespace codablecash */
