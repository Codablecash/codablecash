/*
 * InstanceSpaceReleaser.cpp
 *
 *  Created on: Dec 1, 2025
 *      Author: iizuka
 */

#include "smartcontract_cache/InstanceSpaceReleaser.h"
#include "smartcontract_cache/InstanceSpace.h"


namespace codablecash {

InstanceSpaceReleaser::InstanceSpaceReleaser(InstanceSpace* space) {
	this->space = space;
}

InstanceSpaceReleaser::~InstanceSpaceReleaser() {
	if(this->space != nullptr){
		this->space->decRef();
	}
}

} /* namespace codablecash */
