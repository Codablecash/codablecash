/*
 * BlockHeaderCommand.h
 *
 *  Created on: Jul 23, 2026
 *      Author: iizuka
 */

#ifndef BC_BLOCK_HEADER_COMMAND_BLOCKHEADERCOMMANDID_H_
#define BC_BLOCK_HEADER_COMMAND_BLOCKHEADERCOMMANDID_H_

#include "bc_base/Abstract32BytesId.h"

namespace codablecash {

class BlockHeaderCommandId : public Abstract32BytesId {
public:
	BlockHeaderCommandId(const BlockHeaderCommandId& inst);
	BlockHeaderCommandId();
	BlockHeaderCommandId(const char* binary, int length);
	virtual ~BlockHeaderCommandId();

	static BlockHeaderCommandId* fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;

	static BlockHeaderCommandId* makeRandomHeaderId();
};

} /* namespace codablecash */

#endif /* BC_BLOCK_HEADER_COMMAND_BLOCKHEADERCOMMANDID_H_ */
