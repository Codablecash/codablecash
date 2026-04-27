/*
 * PoWPoolCheckDataCommandResponse.cpp
 *
 *  Created on: Apr 21, 2026
 *      Author: iizuka
 */

#include "pow_pool_client_cmd/PoWPoolCheckDataCommandResponse.h"

#include "pow_pool/PoWRequest2Client.h"
#include "pow_pool/PoWRequestData.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "numeric/BigInteger.h"

#include "base_io/ByteBuffer.h"

#include "bc_base/BinaryUtils.h"

#include "bc_block/BlockHeaderId.h"

#include "base_timestamp/SystemTimestamp.h"

#include "bc_block/BlockMerkleRoot.h"
namespace codablecash {

PoWPoolCheckDataCommandResponse::PoWPoolCheckDataCommandResponse() : AbstractCommandResponse(TYPE_RES_POW_POOL_CLIENT_CHECK_DATA) {
	this->request = nullptr;
}

PoWPoolCheckDataCommandResponse::~PoWPoolCheckDataCommandResponse() {
	delete this->request;
}

int PoWPoolCheckDataCommandResponse::binarySize() const {
	BinaryUtils::checkNotNull(this->request);

	int total = sizeof(this->type);
	total += this->request->binarySize();

	return total;
}

void PoWPoolCheckDataCommandResponse::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->request);

	buff->putInt(this->type);
	this->request->toBinary(buff);
}

void PoWPoolCheckDataCommandResponse::fromBinary(ByteBuffer *buff) {
	this->request = PoWRequest2Client::fromBinary(buff);
}

UnicodeString* PoWPoolCheckDataCommandResponse::toString() const noexcept {
	UnicodeString* ret = new UnicodeString(L""); __STP(ret);

	bool bl = this->request->isWaiting();
	ret->append(L"suspended : ").append(bl ? L"true" : L"false").append(L"\n");

	{
		PoWRequestData* data = this->request->getRequestData();
		uint64_t lastBlockHeight = data->getLastBlockHeight();
		ret->append(L"lastBlockHeight : ").append((int64_t)lastBlockHeight).append(L"\n");

		BlockHeaderId* id = data->getLastBlockId();
		{
			UnicodeString* str = id->toString(); __STP(str);
			ret->append(L"LastBlockId : ").append(str).append(L"\n");
		}

		BlockMerkleRoot *merkleRoot = data->getMerkleRoot();
		{
			UnicodeString* str = merkleRoot->toString(); __STP(str);
			ret->append(L"merkleRoot : ").append(str).append(L"\n");
		}

		SystemTimestamp *tm = data->getTimestamp();
		BigInteger *targetDifficulty = data->getTargetDifficulty();
		{
			BigInteger mills = tm->toBigIntegerMills();
			UnicodeString str = mills.toString();
			ret->append(L"merkleRoot : ").append(&str).append(L"\n");
		}
	}

	{
		BigInteger* s = this->request->getSolt();
		UnicodeString str = s->toString(16);
		ret->append(L"solt : ").append(&str).append(L"\n");
	}

	return __STP_MV(ret);
}

void PoWPoolCheckDataCommandResponse::setRequest(const PoWRequest2Client *request) {
	delete this->request, this->request = nullptr;
	this->request = new PoWRequest2Client(*request);
}

} /* namespace codablecash */
