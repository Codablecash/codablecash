/*
 * PoWNonceCalcCommandMessage.h
 *
 *  Created on: Apr 17, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_POWNONCECALCCOMMANDMESSAGE_H_
#define POW_POOL_CLIENT_POWNONCECALCCOMMANDMESSAGE_H_

#include "procesor/ICommandMessage.h"

namespace alinous {
class BigInteger;
class SystemTimestamp;
}
using namespace alinous;

namespace codablecash {

class PoWRequestData;
class BlockHeaderId;
class BlockMerkleRoot;
class PoWNonceProcessor;

class PoWNonceCalcCommandMessage : public ICommandMessage {
public:
	PoWNonceCalcCommandMessage(const PoWRequestData *data, const BigInteger* solt, int loops, int index);
	virtual ~PoWNonceCalcCommandMessage();

	virtual void execute(ICommandParameter* param);

private:
	bool calcOne(const BlockHeaderId *blockHeaderId, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm, PoWNonceProcessor* processor);

private:
	const PoWRequestData *data;
	BigInteger* solt;
	int loops;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_POWNONCECALCCOMMANDMESSAGE_H_ */
