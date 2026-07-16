/*
 * AbstractShardExtentionValidator.cpp
 *
 *  Created on: Jun 29, 2026
 *      Author: iizuka
 */

#include "bc_status_cache/AbstractShardExtentionValidator.h"

namespace codablecash {

AbstractShardExtentionValidator::AbstractShardExtentionValidator(const AbstractShardExtentionValidator &inst) {
	this->statusCache = inst.statusCache;
}

AbstractShardExtentionValidator::AbstractShardExtentionValidator() {
	this->statusCache = nullptr;
}

AbstractShardExtentionValidator::~AbstractShardExtentionValidator() {
	this->statusCache = nullptr;
}

void AbstractShardExtentionValidator::setStatusCache(BlockchainStatusCache *stcache) noexcept {
	this->statusCache = stcache;
}

} /* namespace codablecash */
