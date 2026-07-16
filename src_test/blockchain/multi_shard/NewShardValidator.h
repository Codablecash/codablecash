/*
 * NewShardValidator.h
 *
 *  Created on: Jun 30, 2026
 *      Author: iizuka
 */

#ifndef BLOCKCHAIN_MULTI_SHARD_NEWSHARDVALIDATOR_H_
#define BLOCKCHAIN_MULTI_SHARD_NEWSHARDVALIDATOR_H_

#include "bc_status_cache/AbstractShardExtentionValidator.h"

namespace codablecash {

class NewShardValidator: public AbstractShardExtentionValidator {
public:
	NewShardValidator(const NewShardValidator& inst);
	NewShardValidator();
	virtual ~NewShardValidator();

	virtual bool validate(NewShardZoneCommand* newShardCommand, IStatusCacheContext* context, BlockchainController* ctrl);

	virtual AbstractShardExtentionValidator* copy() const;
};

} /* namespace codablecash */

#endif /* BLOCKCHAIN_MULTI_SHARD_NEWSHARDVALIDATOR_H_ */
