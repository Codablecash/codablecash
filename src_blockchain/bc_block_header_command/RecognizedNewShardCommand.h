/*
 * RecognizedNewShardCommand.h
 *
 *  Created on: Jul 4, 2026
 *      Author: iizuka
 */

#ifndef BC_BLOCK_HEADER_COMMAND_RECOGNIZEDNEWSHARDCOMMAND_H_
#define BC_BLOCK_HEADER_COMMAND_RECOGNIZEDNEWSHARDCOMMAND_H_

#include "bc_block_header_command/AbstractBlockHeaderCommand.h"

namespace codablecash {

class RecognizedNewShardCommand : public AbstractBlockHeaderCommand {
public:
	RecognizedNewShardCommand(const RecognizedNewShardCommand& inst);
	RecognizedNewShardCommand();
	virtual ~RecognizedNewShardCommand();

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	virtual void onFinalize(const BlockHeader *header, BlockchainStatusCache* statusCache, CodablecashBlockchain* blockchain, ILockinManager *lockinManager, const CodablecashSystemParam* config);

};

} /* namespace codablecash */

#endif /* BC_BLOCK_HEADER_COMMAND_RECOGNIZEDNEWSHARDCOMMAND_H_ */
