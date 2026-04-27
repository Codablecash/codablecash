/*
 * BlockHeaderId.h
 *
 *  Created on: 2023/02/16
 *      Author: iizuka
 */

#ifndef BC_BLOCK_BLOCKHEADERID_H_
#define BC_BLOCK_BLOCKHEADERID_H_

#include "bc_base/Abstract32BytesId.h"

using namespace alinous;

namespace codablecash {

class BlockHeaderId : public Abstract32BytesId {
public:
	BlockHeaderId(const BlockHeaderId& inst);
	BlockHeaderId();
	BlockHeaderId(const char* binary, int length);
	virtual ~BlockHeaderId();

	static BlockHeaderId* makeRandomHeaderId();

	static BlockHeaderId* fromBinary(ByteBuffer* in);

	virtual IBlockObject* copyData() const noexcept;
};

} /* namespace codablecash */

#endif /* BC_BLOCK_BLOCKHEADERID_H_ */
