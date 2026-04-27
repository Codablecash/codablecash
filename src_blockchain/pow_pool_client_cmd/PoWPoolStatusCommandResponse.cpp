/*
 * PoWPoolStatusCommandResponse.cpp
 *
 *  Created on: Apr 20, 2026
 *      Author: iizuka
 */

#include "pow_pool_client_cmd/PoWPoolStatusCommandResponse.h"

#include "pow_pool/PoWRequestStatusData.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "bc_base/BinaryUtils.h"


using namespace alinous;

namespace codablecash {

PoWPoolStatusCommandResponse::PoWPoolStatusCommandResponse() : AbstractCommandResponse(TYPE_RES_POW_POOL_CLIENT_STATUS) {
	this->data = nullptr;
}

PoWPoolStatusCommandResponse::~PoWPoolStatusCommandResponse() {
	delete this->data;
}

int PoWPoolStatusCommandResponse::binarySize() const {
	BinaryUtils::checkNotNull(this->data);

	int total = sizeof(this->type);
	total += this->data->binarySize();

	return total;
}

void PoWPoolStatusCommandResponse::toBinary(ByteBuffer *buff) const {
	BinaryUtils::checkNotNull(this->data);

	buff->putInt(this->type);
	this->data->toBinary(buff);
}

void PoWPoolStatusCommandResponse::fromBinary(ByteBuffer *buff) {
	this->data = PoWRequestStatusData::fromBinary(buff);
}

UnicodeString* PoWPoolStatusCommandResponse::toString() const noexcept {
	UnicodeString* ret = new UnicodeString(L""); __STP(ret);

	ret->append(L"Suspend : ");
	bool bl = this->data->isWaiting();
	ret->append(bl ? L"true" : L"false");

	return __STP_MV(ret);
}

void PoWPoolStatusCommandResponse::setData(const PoWRequestStatusData *data) {
	delete this->data;
	this->data = new PoWRequestStatusData(*data);
}

} /* namespace codablecash */
