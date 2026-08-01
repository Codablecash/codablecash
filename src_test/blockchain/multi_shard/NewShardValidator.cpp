/*
 * NewShardValidator.cpp
 *
 *  Created on: Jun 30, 2026
 *      Author: iizuka
 */

#include "NewShardValidator.h"

#include <cstdint>

#include "bc_status_cache_context/IStatusCacheContext.h"

#include "bc_block_header_command/AbstractBlockHeaderCommand.h"
#include "bc_block_header_command/RecognizedNewShardCommand.h"

namespace codablecash {

NewShardValidator::NewShardValidator(const NewShardValidator &inst) : AbstractShardExtentionValidator(inst) {
}

NewShardValidator::NewShardValidator() : AbstractShardExtentionValidator() {

}

NewShardValidator::~NewShardValidator() {

}

bool NewShardValidator::validate(AbstractBlockHeaderCommand *headerCommand, IStatusCacheContext *context, BlockchainController *ctrl) {
	uint16_t t = headerCommand->getType();

	if(t == AbstractBlockHeaderCommand::RECOGNIZED_SHARD_COMMAND){
		RecognizedNewShardCommand* rcmd = dynamic_cast<RecognizedNewShardCommand*>(headerCommand);
		return validateRecognizedNewShardCommand(rcmd, context, ctrl);
	}

	uint16_t numZones = context->getNumZones();
	int requestedZones = context->getRequestedNewShards();

	return requestedZones == 0;
}

AbstractShardExtentionValidator* NewShardValidator::copy() const {
	return new NewShardValidator(*this);
}

} /* namespace codablecash */
