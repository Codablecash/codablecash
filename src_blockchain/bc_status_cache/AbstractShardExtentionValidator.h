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
class AbstractBlockHeaderCommand;
class RecognizedNewShardCommand;

class AbstractShardExtentionValidator {
public:
	AbstractShardExtentionValidator(const AbstractShardExtentionValidator& inst);
	AbstractShardExtentionValidator();
	virtual ~AbstractShardExtentionValidator();

	virtual AbstractShardExtentionValidator* copy() const = 0;

	void setStatusCache(BlockchainStatusCache* stcache) noexcept;

	virtual bool validate(AbstractBlockHeaderCommand* blockHeaderCommand, IStatusCacheContext* context, BlockchainController* ctrl) = 0;

protected:
	virtual bool validateRecognizedNewShardCommand(RecognizedNewShardCommand* recognizeNewShardCommand, IStatusCacheContext* context, BlockchainController* ctrl);

private:
	BlockchainStatusCache* statusCache;
};

} /* namespace codablecash */

#endif /* BC_STATUS_CACHE_ABSTRACTSHARDEXTENTIONVALIDATOR_H_ */
