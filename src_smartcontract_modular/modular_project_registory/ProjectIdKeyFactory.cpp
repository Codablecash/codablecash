/*
 * ProjectIdKeyFactory.cpp
 *
 *  Created on: Nov 27, 2025
 *      Author: iizuka
 */

#include "modular_project_registory/ProjectIdKeyFactory.h"
#include "modular_project_registory/ProjectIdKey.h"


namespace codablecash {

ProjectIdKeyFactory::ProjectIdKeyFactory() {


}

ProjectIdKeyFactory::~ProjectIdKeyFactory() {

}

AbstractBtreeKey* ProjectIdKeyFactory::fromBinary(uint32_t keyType, ByteBuffer *in) const {
	if(keyType == PROJECT_ID_KEY){
		return ProjectIdKey::fromBinary(in);
	}

	return BtreeKeyFactory::fromBinary(keyType, in);
}

BtreeKeyFactory* ProjectIdKeyFactory::copy() const noexcept {
	return new ProjectIdKeyFactory();
}

} /* namespace codablecash */
