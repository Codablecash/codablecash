/*
 * ProjectId.cpp
 *
 *  Created on: Nov 27, 2025
 *      Author: iizuka
 */

#include "modular_project/SmartcontractProjectId.h"

#include "base/StackRelease.h"

namespace codablecash {

SmartcontractProjectId::SmartcontractProjectId(const SmartcontractProjectId &inst) : Abstract32BytesId(inst) {

}

SmartcontractProjectId::SmartcontractProjectId() {

}

SmartcontractProjectId::SmartcontractProjectId(const char *binary, int length) : Abstract32BytesId(binary, length) {
}

SmartcontractProjectId::~SmartcontractProjectId() {

}

int SmartcontractProjectId::binarySize() const {
	return Abstract32BytesId::binarySize();
}

void SmartcontractProjectId::toBinary(ByteBuffer *out) const {
	Abstract32BytesId::toBinary(out);
}

SmartcontractProjectId* SmartcontractProjectId::fromBinary(ByteBuffer *in) {
	SmartcontractProjectId* projectId = new SmartcontractProjectId();

	int cap = in->getInt();

	uint8_t* buff = new uint8_t[cap];
	StackArrayRelease<uint8_t> __st_buff(buff);

	in->get(buff, cap);

	projectId->id = ByteBuffer::wrapWithEndian(buff, cap, true);
	projectId->id->position(0);

	return projectId;
}

IBlockObject* SmartcontractProjectId::copyData() const noexcept {
	return new SmartcontractProjectId(*this);
}

} /* namespace codablecash */
