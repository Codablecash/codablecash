/*
 * NewShardValidator.cpp
 *
 *  Created on: Jun 30, 2026
 *      Author: iizuka
 */

#include "NewShardValidator.h"

#include <cstdint>

#include "bc_status_cache_context/IStatusCacheContext.h"

namespace codablecash {

NewShardValidator::NewShardValidator(const NewShardValidator &inst) : AbstractShardExtentionValidator(inst) {
}

NewShardValidator::NewShardValidator() : AbstractShardExtentionValidator() {

}

NewShardValidator::~NewShardValidator() {

}

bool NewShardValidator::validate(NewShardZoneCommand *newShardCommand, IStatusCacheContext *context, BlockchainController *ctrl) {
	uint16_t numZones = context->getNumZones();
	int requestedZones = context->getRequestedNewShards();

	return requestedZones == 0;
}

AbstractShardExtentionValidator* NewShardValidator::copy() const {
	return new NewShardValidator(*this);
}

} /* namespace codablecash */
