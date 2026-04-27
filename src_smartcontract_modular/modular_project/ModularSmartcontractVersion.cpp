/*
 * ModularSmartcontractVersion.cpp
 *
 *  Created on: Oct 4, 2025
 *      Author: iizuka
 */

#include "modular_project/ModularSmartcontractVersion.h"
#include "modular_project/ModularConfigException.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

#include "bc/SoftwareVersion.h"
#include "bc/ExceptionThrower.h"

#include "base_io/ByteBuffer.h"

using namespace alinous;

namespace codablecash {

ModularSmartcontractVersion::ModularSmartcontractVersion(const ModularSmartcontractVersion &inst)
		: SoftwareVersion(inst.major, inst.minor, inst.patch) {
	this->versionType = inst.versionType;
}

ModularSmartcontractVersion::ModularSmartcontractVersion(int major, int minor, int patch, int verType)
		: SoftwareVersion(major, minor, patch) {
	this->versionType = verType;
}

ModularSmartcontractVersion::~ModularSmartcontractVersion() {

}

bool ModularSmartcontractVersion::validate(const SoftwareVersion *other) const noexcept {
	if(this->versionType == TYPE_TILDE){
		return validateTilde(other);
	}

	return validateCaret(other);
}

bool ModularSmartcontractVersion::validateCaret(const SoftwareVersion *other) const noexcept {
	SoftwareVersion* maxVar = maxCaretVersion(other); __STP(maxVar);

	return other->compareTo(this) >= 0 && other->compareTo(maxVar) < 0;
}

SoftwareVersion* ModularSmartcontractVersion::maxCaretVersion(const SoftwareVersion *other) const noexcept {
	int ma = 0;
	int mi = 0;
	int pat = 0;

	if(this->getMajor() == 0){
		if(other->getMinor() == 0){
			ma = 0;
			mi = 0;
			pat = this->getPatch() + 1;
		}
		else {
			ma = 0;
			mi = this->getMinor() + 1;
			pat = 0;
		}
	}
	else{
		ma = this->getMajor() + 1;
		mi = 0;
		pat = 0;
	}

	return new SoftwareVersion(ma, mi, pat);
}

bool ModularSmartcontractVersion::validateTilde(const SoftwareVersion *other) const noexcept {
	int ma = other->getMajor();
	int mi = other->getMinor() + 1;
	int pat = 0;

	SoftwareVersion maxVer(ma, mi, pat);

	return other->compareTo(this) >= 0 && other->compareTo(&maxVer) < 0;;
}

ModularSmartcontractVersion* ModularSmartcontractVersion::parseString(const UnicodeString *str) {
	int verType = 0;

	wchar_t ch = str->charAt(0);

	ExceptionThrower<ModularConfigException>::throwExceptionIfCondition(ch != L'~' && ch != L'^', L"Invalid version specification format.", __FILE__, __LINE__);

	if(ch == L'~'){
		verType = TYPE_TILDE;
	}
	else{
		verType = TYPE_CARET;
	}

	UnicodeString* verstr = str->substring(1); __STP(verstr);

	SoftwareVersion* version = SoftwareVersion::parseString(verstr); __STP(version);

	return new ModularSmartcontractVersion(version->getMajor(), version->getMinor(), version->getPatch(), verType);
}

int ModularSmartcontractVersion::binarySize() const {
	int total = SoftwareVersion::binarySize();
	total += sizeof(uint8_t);

	return total;
}

void ModularSmartcontractVersion::toBinary(ByteBuffer *out) const {
	SoftwareVersion::toBinary(out);
	out->put(this->versionType);
}

ModularSmartcontractVersion* ModularSmartcontractVersion::createFromBinary(ByteBuffer *in) {
	uint8_t major = in->get();
	uint8_t minor = in->get();
	uint8_t pathch = in->get();
	uint8_t versionType = in->get();

	return new ModularSmartcontractVersion(major, minor, pathch, versionType);
}

} /* namespace codablecash */
