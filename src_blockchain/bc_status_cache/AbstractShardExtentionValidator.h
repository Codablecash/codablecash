/*
 * AbstractShardExtentionValidator.h
 *
 *  Created on: Jun 29, 2026
 *      Author: iizuka
 */

#ifndef BC_STATUS_CACHE_ABSTRACTSHARDEXTENTIONVALIDATOR_H_
#define BC_STATUS_CACHE_ABSTRACTSHARDEXTENTIONVALIDATOR_H_

namespace codablecash {

class BlockchainStatusCache;
class IStatusCacheContext;
class BlockchainController;
class NewShardZoneCommand;


class AbstractShardExtentionValidator {
public:
	AbstractShardExtentionValidator(const AbstractShardExtentionValidator& inst);
	AbstractShardExtentionValidator();
	virtual ~AbstractShardExtentionValidator();

	virtual AbstractShardExtentionValidator* copy() const = 0;

	void setStatusCache(BlockchainStatusCache* stcache) noexcept;

	virtual bool validate(NewShardZoneCommand* newShardCommand, IStatusCacheContext* context, BlockchainController* ctrl) = 0;

private:
	BlockchainStatusCache* statusCache;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_ABSTRACTSHARDEXTENTIONVALIDATOR_H_ */
