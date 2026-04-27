/*
 * SoftwareVersion.cpp
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#include "bc/SoftwareVersion.h"

#include "base/UnicodeString.h"
#include "base/ArrayList.h"
#include "base/StackRelease.h"
#include "base/Integer.h"

#include "base_io/ByteBuffer.h"


namespace codablecash {

SoftwareVersion::SoftwareVersion(const SoftwareVersion &inst) {
	this->major = inst.major;
	this->minor = inst.minor;
	this->patch = inst.patch;
}

SoftwareVersion::SoftwareVersion(int major, int minor, int patch) {
	this->major = major;
	this->minor = minor;
	this->patch = patch;
}

SoftwareVersion::~SoftwareVersion() {

}

uint64_t SoftwareVersion::toLongValue() const noexcept {
	uint64_t v = this->major;
	v = v << 16;
	v = v + this->minor;
	v = v << 16;
	v = v + this->patch;

	return v;
}

int SoftwareVersion::compareTo(const SoftwareVersion *other) const noexcept {
	int diff = this->major - other->major;
	if(diff != 0){
		return diff;
	}

	diff = this->minor - other->minor;
	if(diff != 0){
		return diff;
	}

	diff = this->patch - other->patch;
	return diff;
}

SoftwareVersion* SoftwareVersion::parseString(const UnicodeString *str) {
	int major = 0;
	int minor = 0;
	int patch = 0;

	UnicodeString pattern(L"\\.");
	ArrayList<UnicodeString>* list = str->split(&pattern); __STP(list);
	list->setDeleteOnExit();

	int length =list->size();
	if(length > 0){
		UnicodeString* v = list->get(0);
		major = Integer::parseInt(v, 10);
	}
	if(length > 1){
		UnicodeString* v = list->get(1);
		minor = Integer::parseInt(v, 10);
	}
	if(length > 2){
		UnicodeString* v = list->get(2);
		patch = Integer::parseInt(v, 10);
	}

	return new SoftwareVersion(major, minor, patch);
}

int SoftwareVersion::binarySize() const {
	return sizeof(uint8_t) * 3;
}

void SoftwareVersion::toBinary(ByteBuffer *out) const {
	out->put(this->major);
	out->put(this->minor);
	out->put(this->patch);
}

SoftwareVersion* SoftwareVersion::createFromBinary(ByteBuffer *in) {
	uint8_t major = in->get();
	uint8_t minor = in->get();
	uint8_t pathch = in->get();

	return new SoftwareVersion(major, minor, pathch);
}

} /* namespace codablecash */
