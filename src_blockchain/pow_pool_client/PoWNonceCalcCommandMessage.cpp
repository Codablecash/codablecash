/*
 * PoWNonceCalcCommandMessage.cpp
 *
 *  Created on: Apr 17, 2026
 *      Author: iizuka
 */

#include "pow_pool_client/PoWNonceCalcCommandMessage.h"
#include "pow_pool_client/PoWNonceProcessorParam.h"
#include "pow_pool_client/PoWNonceProcessor.h"

#include "pow_pool/PoWRequestData.h"

#include "pow/PoWNonce.h"
#include "pow/PoWNonceResult.h"

#include "base/StackRelease.h"

#include "numeric/BigInteger.h"

#include "bc_block/BlockHeaderId.h"


namespace codablecash {

PoWNonceCalcCommandMessage::PoWNonceCalcCommandMessage(const PoWRequestData *data, const BigInteger* solt, int loops, int index) {
	this->data = data;
	this->loops = loops;

	BigInteger val(index);
	BigInteger newSolt = solt->add(val);
	this->solt = new BigInteger(newSolt);
}

PoWNonceCalcCommandMessage::~PoWNonceCalcCommandMessage() {
	delete this->solt;
}

void PoWNonceCalcCommandMessage::execute(ICommandParameter *param) {
	PoWNonceProcessorParam* nparam = dynamic_cast<PoWNonceProcessorParam*>(param);
	PoWWorkerClient* client = nparam->getClient();
	PoWNonceProcessor* processor = nparam->getProcessor();

	const BlockHeaderId* blockHeaderId = this->data->getLastBlockId();
	const BlockMerkleRoot* merkleRoot = this->data->getMerkleRoot();
	const SystemTimestamp* tm = this->data->getTimestamp();

	int maxLoop = this->loops;
	for(int i = 0; i != maxLoop; ++i){
		bool bl = calcOne(blockHeaderId, merkleRoot, tm, processor);
		if(!bl){
			break;
		}
	}
}

bool PoWNonceCalcCommandMessage::calcOne(const BlockHeaderId *blockHeaderId, const BlockMerkleRoot* merkleRoot, const SystemTimestamp* tm, PoWNonceProcessor* processor) {
	PoWNonce* n = PoWNonce::createRandomNonce(this->solt); __STP(n);

	PoWNonceResult* result = n->calcResult(blockHeaderId, merkleRoot, tm); __STP(result);
	BigInteger diff = result->getDifficulty();

	return processor->onNonceCalculated(n, &diff);
}

} /* namespace codablecash */
