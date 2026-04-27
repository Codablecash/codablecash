/*
 * InstanceSpaceReleaser.h
 *
 *  Created on: Dec 1, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_CACHE_INSTANCESPACERELEASER_H_
#define SMARTCONTRACT_CACHE_INSTANCESPACERELEASER_H_

namespace codablecash {

class InstanceSpace;

class InstanceSpaceReleaser {
public:
	explicit InstanceSpaceReleaser(InstanceSpace* space);
	virtual ~InstanceSpaceReleaser();

private:
	InstanceSpace* space;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_CACHE_INSTANCESPACERELEASER_H_ */
