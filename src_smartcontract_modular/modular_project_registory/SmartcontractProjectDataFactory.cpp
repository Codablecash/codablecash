/*
 * SmartcontractProjectDataFactory.cpp
 *
 *  Created on: Nov 27, 2025
 *      Author: iizuka
 */

#include "modular_project_registory/SmartcontractProjectDataFactory.h"
#include "modular_project_registory/SmartcontractProjectData.h"


namespace codablecash {

SmartcontractProjectDataFactory::SmartcontractProjectDataFactory() {


}

SmartcontractProjectDataFactory::~SmartcontractProjectDataFactory() {

}

IBlockObject* SmartcontractProjectDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return SmartcontractProjectData::createFromBinary(in);
}

AbstractBtreeDataFactory* SmartcontractProjectDataFactory::copy() const noexcept {
	return new SmartcontractProjectDataFactory();
}

} /* namespace codablecash */
