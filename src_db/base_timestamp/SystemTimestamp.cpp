/*
 * SystemTimestamp.cpp
 *
 *  Created on: 2023/12/27
 *      Author: iizuka
 */

#include "base_timestamp/SystemTimestamp.h"

#include "osenv/funcs.h"

#include "base_io/ByteBuffer.h"

#include "numeric/BigInteger.h"

namespace alinous {

SystemTimestamp::SystemTimestamp(const SystemTimestamp &inst) {
	this->sec = inst.sec;
	this->usec = inst.usec;
}

SystemTimestamp::SystemTimestamp() {
	SystemTimestamp now = Os::now();

	this->sec = now.sec;
	this->usec = now.usec;
}

SystemTimestamp::SystemTimestamp(uint64_t sec, uint32_t usec) {
	this->sec = sec;
	this->usec = usec;

	adjustUsec();
}

SystemTimestamp::~SystemTimestamp() {

}

SystemTimestamp& SystemTimestamp::operator =(const SystemTimestamp &o) {
	if(&o != this){
		this->sec = o.sec;
		this->usec = o.usec;
	}
	return *this;
}

SystemTimestamp SystemTimestamp::operator +(const SystemTimestamp &o) const {
	uint64_t newSec = this->sec + o.sec;
	uint32_t newUsec = this->usec + o.usec;

	return SystemTimestamp(newSec, newUsec);
}

SystemTimestamp SystemTimestamp::operator -(const SystemTimestamp &o) const {
	uint64_t newSec = this->sec - o.sec;

	uint32_t newUsec;
	if(this->usec >= o.usec){
		newUsec = this->usec - o.usec;
	}
	else{
		newSec--;
		newUsec = MAX_USEC + this->usec - o.usec;
	}

	return SystemTimestamp(newSec, newUsec);
}

int SystemTimestamp::binarySize() const {
	int total = sizeof(this->sec);
	total += sizeof(this->usec);

	return total;
}

void SystemTimestamp::toBinary(ByteBuffer *out) const {
	out->putLong(this->sec);
	out->putInt(this->usec);
}

SystemTimestamp* SystemTimestamp::fromBinary(ByteBuffer *in) {
	uint64_t sec = in->getLong();
	uint32_t usec = in->getInt();

	return new SystemTimestamp(sec, usec);
}

IBlockObject* SystemTimestamp::copyData() const noexcept {
	return new SystemTimestamp(*this);
}

void SystemTimestamp::adjustUsec() noexcept {
	while(this->usec >= MAX_USEC){
		this->sec++;
		this->usec -= MAX_USEC;
	}
}

int SystemTimestamp::compareTo(const SystemTimestamp* other) const noexcept {
	int diff = this->sec - other->sec;
	if(diff != 0){
		return diff;
	}

	return this->usec == other->usec ? 0 : (this->usec > other->usec ? 1 : -1);
	// return diff != 0 ? diff : this->usec - other->usec;
}

BigInteger SystemTimestamp::toBigIntegerMills() const noexcept {
	BigInteger sec(this->sec);
	BigInteger mill(this->usec / 1000);
	BigInteger thousand(1000);

	mill.addSelf(sec.multiply(thousand));

	return mill;
}

bool SystemTimestamp::isZero() const noexcept {
	return this->sec == 0L && this->usec == 0;
}

} /* namespace alinous */
