/*
 * ProjectIndexDataFactory.cpp
 *
 *  Created on: Feb 1, 2026
 *      Author: iizuka
 */

#include "smartcontract_executor_index_projectid/ProjectIndexDataFactory.h"
#include "smartcontract_executor_index_projectid/ProjectIndexData.h"

namespace codablecash {

ProjectIndexDataFactory::ProjectIndexDataFactory() {

}

ProjectIndexDataFactory::~ProjectIndexDataFactory() {

}

IBlockObject* ProjectIndexDataFactory::makeDataFromBinary(ByteBuffer *in) {
	return ProjectIndexData::createFromBinary(in);
}

AbstractBtreeDataFactory* ProjectIndexDataFactory::copy() const noexcept {
	return new ProjectIndexDataFactory();
}

} /* namespace codablecash */
