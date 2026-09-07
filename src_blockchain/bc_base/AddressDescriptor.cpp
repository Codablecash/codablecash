/*
 * AddressDescriptor.cpp
 *
 *  Created on: 2023/01/24
 *      Author: iizuka
 */

#include "bc_base/AddressDescriptor.h"
#include "bc_base/Base58.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "base_io/ByteBuffer.h"

#include "osenv/memory.h"

#include <cstdio>

#include "bc/ExceptionThrower.h"

#include "bc_base/AddressCheckDigitException.h"

#include "base/Integer.h"
using namespace alinous;

namespace codablecash {

AddressDescriptor::AddressDescriptor(const AddressDescriptor &inst) {
	Mem::memcpy(this->prefix, inst.prefix, AddressDescriptor::PREFIX_LENGTH);
	Mem::memcpy(this->zone, inst.zone, AddressDescriptor::ZONE_LENGTH);
	this->body = inst.body->clone();

	makeCheckDigit();
}

AddressDescriptor::AddressDescriptor(const char *prefix, const char *zone, const char *body, int bodylength) {
	Mem::memcpy(this->prefix, prefix, AddressDescriptor::PREFIX_LENGTH);
	Mem::memcpy(this->zone, zone, AddressDescriptor::ZONE_LENGTH);
	this->body = ByteBuffer::wrapWithEndian(body, bodylength, true);

	makeCheckDigit();
}

AddressDescriptor::AddressDescriptor(const char *cstr) {
	importCstring(cstr);
}

AddressDescriptor::AddressDescriptor(const UnicodeString *str) {
	const char* cstr = str->toCString();
	StackArrayRelease<const char> __st_cstr(cstr);

	importCstring(cstr);
}

AddressDescriptor::~AddressDescriptor() {
	delete this->body;
}

const char* AddressDescriptor::toCString() const noexcept {
	UnicodeString* str = Base58::encode((const char*)this->body->array(), this->body->limit()); __STP(str);

	const char* bodyCstr = str->toCString();
	StackArrayRelease<const char> __st_bodycstr(bodyCstr);

	int bodylength = str->length();
	int capacity = AddressDescriptor::PREFIX_LENGTH + AddressDescriptor::ZONE_LENGTH
			+ bodylength + AddressDescriptor::CHECKDIGIT_LENGTH;
	ByteBuffer* buff = ByteBuffer::allocateWithEndian(capacity, true); __STP(buff);

	buff->put((const uint8_t*)this->prefix, AddressDescriptor::PREFIX_LENGTH);
	buff->put((const uint8_t*)this->zone, AddressDescriptor::ZONE_LENGTH);

	buff->put((const uint8_t*)bodyCstr, bodylength);
	buff->put((const uint8_t*)this->checkDigit, AddressDescriptor::CHECKDIGIT_LENGTH);

	buff->position(0);

	char* ret = new char[capacity + 1];
	Mem::memset(ret, 0, capacity + 1);
	Mem::memcpy(ret, buff->array(), buff->limit());

	return ret;
}

void AddressDescriptor::importCstring(const char *cstr) {
	int length = Mem::strlen(cstr);

	Mem::memcpy(this->prefix, cstr, AddressDescriptor::PREFIX_LENGTH);
	Mem::memcpy(this->zone, cstr + AddressDescriptor::PREFIX_LENGTH, AddressDescriptor::ZONE_LENGTH);

	int bodylength = length - AddressDescriptor::PREFIX_LENGTH - AddressDescriptor::ZONE_LENGTH
			- AddressDescriptor::CHECKDIGIT_LENGTH;
	char* bodycstr = new char[bodylength + 1];
	Mem::memset(bodycstr, 0, bodylength + 1);
	StackArrayRelease<const char> __st_bodycstr(bodycstr);

	Mem::memcpy(bodycstr, cstr + AddressDescriptor::PREFIX_LENGTH + AddressDescriptor::ZONE_LENGTH, bodylength);

	UnicodeString str(bodycstr);
	this->body = Base58::decode(&str);

	// check checkdigits
	char __checkDigit[2];
	Mem::memcpy(__checkDigit, cstr + AddressDescriptor::PREFIX_LENGTH + AddressDescriptor::ZONE_LENGTH + bodylength, AddressDescriptor::CHECKDIGIT_LENGTH);

	makeCheckDigit();

	int cmp = Mem::memcmp(this->checkDigit, __checkDigit, AddressDescriptor::CHECKDIGIT_LENGTH);
	ExceptionThrower<AddressCheckDigitException>::throwExceptionIfCondition(cmp != 0, L"Check digit error.", __FILE__, __LINE__);
}

void AddressDescriptor::makeCheckDigit() {
	this->body->position(0);
	const uint8_t* bodybin = this->body->array();

	uint64_t total = 0;

	int maxLoop = this->body->limit();
	for(int i = 0; i != maxLoop; ++i){
		total += bodybin[i];
	}

	int checkdigit = total % 99;
	char tmp[3];
	Mem::memset(tmp, 0, 3);

	::sprintf(tmp, "%02d", checkdigit);

	Mem::memcpy(this->checkDigit, tmp, 2);
}

int AddressDescriptor::compareTo(const AddressDescriptor *other) const noexcept {
	const char* thiscstr = toCString();
	const char* othercstr = other->toCString();
	StackArrayRelease<const char> __st_thiscstr(thiscstr);
	StackArrayRelease<const char> __st_othercstr(othercstr);

	return Mem::strcmp(thiscstr, othercstr);
}

int AddressDescriptor::binarySize() const {
	int total = sizeof(char) * 5;

	total += sizeof(uint8_t);
	total += this->body->limit();

	return total;
}

void AddressDescriptor::toBinary(ByteBuffer *out) const {
	out->put((const uint8_t*)this->prefix, AddressDescriptor::PREFIX_LENGTH);
	out->put((const uint8_t*)this->zone, AddressDescriptor::ZONE_LENGTH);

	this->body->position(0);
	int size = this->body->limit();
	out->put(size);

	out->put(this->body);
}

AddressDescriptor* AddressDescriptor::createFromBinary(ByteBuffer *in) {
	char prefix[AddressDescriptor::PREFIX_LENGTH];
	char zone[AddressDescriptor::ZONE_LENGTH];

	in->get((uint8_t*)prefix, AddressDescriptor::PREFIX_LENGTH);
	in->get((uint8_t*)zone, AddressDescriptor::ZONE_LENGTH);

	uint8_t length = in->get();
	uint8_t* dest = new uint8_t[length];
	StackArrayRelease<uint8_t> __st_dest(dest);

	in->get(dest, length);

	return new AddressDescriptor(prefix, zone, (const char*)dest, length);
}

IBlockObject* AddressDescriptor::copyData() const noexcept {
	return new AddressDescriptor(*this);
}

uint16_t AddressDescriptor::getZone() const noexcept {
	char buff[4]{};

	Mem::memcpy(buff, this->prefix, 3);
	UnicodeString str(buff);

	uint16_t zone = Integer::parseInt(&str, 16);
	return zone;
}

} /* namespace codablecash */
