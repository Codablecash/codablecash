/*
 * RecognizedNewShardCommand.cpp
 *
 *  Created on: Jul 4, 2026
 *      Author: iizuka
 */

#include "bc_block_header_command/RecognizedNewShardCommand.h"

namespace codablecash {

RecognizedNewShardCommand::RecognizedNewShardCommand(const RecognizedNewShardCommand &inst) : AbstractBlockHeaderCommand(inst) {
}

RecognizedNewShardCommand::RecognizedNewShardCommand() : AbstractBlockHeaderCommand(AbstractBlockHeaderCommand::RECOGNIZED_SHARD_COMMAND) {
	// TODO Auto-generated constructor stub

}

RecognizedNewShardCommand::~RecognizedNewShardCommand() {

}

int RecognizedNewShardCommand::binarySize() const {
	int total = sizeof(uint16_t);

	return total;
}

void RecognizedNewShardCommand::toBinary(ByteBuffer *out) const {
}

void RecognizedNewShardCommand::fromBinary(ByteBuffer *in) {
}

IBlockObject* RecognizedNewShardCommand::copyData() const noexcept {
	return new RecognizedNewShardCommand(*this);
}

void RecognizedNewShardCommand::onFinalize(const BlockHeader *header,BlockchainStatusCache *statusCache, CodablecashBlockchain *blockchain,
		ILockinManager *lockinManager, const CodablecashSystemParam *config) {
}

} /* namespace codablecash */
