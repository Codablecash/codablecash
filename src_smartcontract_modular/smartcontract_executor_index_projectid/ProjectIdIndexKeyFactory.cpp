/*
 * ProjectIdIndexKeyFactory.cpp
 *
 *  Created on: Jan 29, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_index_projectid/ProjectIdIndexKeyFactory.h"
#include "smartcontract_executor_index_projectid/ProjectIdIndexKey.h"
#include "smartcontract_executor_index_projectid/SmartcontractVersionKey.h"

namespace codablecash {

ProjectIdIndexKeyFactory::ProjectIdIndexKeyFactory() {

}

ProjectIdIndexKeyFactory::~ProjectIdIndexKeyFactory() {

}

AbstractBtreeKey* ProjectIdIndexKeyFactory::fromBinary(uint32_t keyType, ByteBuffer *in) const {
	if(keyType == PROJECT_ID_INDEX_KEY){
		return ProjectIdIndexKey::fromBinary(in);
	}
	else if(keyType == SMARTCONTRACT_VERSION_KEY){
		return SmartcontractVersionKey::fromBinary(in);
	}

	return BtreeKeyFactory::fromBinary(keyType, in);
}

BtreeKeyFactory* ProjectIdIndexKeyFactory::copy() const noexcept {
	return new ProjectIdIndexKeyFactory();
}

} /* namespace codablecash */
