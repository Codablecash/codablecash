/*
 * ModularSmartcontractVersion.h
 *
 *  Created on: Oct 4, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_MODULARSMARTCONTRACTVERSION_H_
#define MODULAR_PROJECT_MODULARSMARTCONTRACTVERSION_H_

#include "bc/SoftwareVersion.h"


namespace codablecash {

class ModularSmartcontractVersion : public SoftwareVersion {
public:
	static const constexpr int TYPE_CARET{1};
	static const constexpr int TYPE_TILDE{2};

	ModularSmartcontractVersion(const ModularSmartcontractVersion& inst);
	ModularSmartcontractVersion(int major, int minor, int patch, int verType);
	virtual ~ModularSmartcontractVersion();

	bool validate(const SoftwareVersion* other) const noexcept;

	static ModularSmartcontractVersion* parseString(const UnicodeString* str);

	int getVersionType() const noexcept {
		return this->versionType;
	}

	// binary
	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static ModularSmartcontractVersion* createFromBinary(ByteBuffer* in);

private:
	bool validateCaret(const SoftwareVersion* other) const noexcept;
	SoftwareVersion* maxCaretVersion(const SoftwareVersion* other) const noexcept;

	bool validateTilde(const SoftwareVersion* other) const noexcept;


private:
	int versionType;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_MODULARSMARTCONTRACTVERSION_H_ */
