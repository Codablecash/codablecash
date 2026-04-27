/*
 * SoftwareVersion.h
 *
 *  Created on: 2023/01/25
 *      Author: iizuka
 */

#ifndef BC_SOFTWAREVERSION_H_
#define BC_SOFTWAREVERSION_H_

#include <cstdint>

namespace alinous {
class UnicodeString;
class ByteBuffer;
}
using namespace alinous;

namespace codablecash {

class SoftwareVersion {
public:
	SoftwareVersion(const SoftwareVersion& inst);
	SoftwareVersion(int major, int minor, int patch);
	virtual ~SoftwareVersion();

	int getMajor() const noexcept {
		return major;
	}
	int getMinor() const noexcept {
		return minor;
	}
	int getPatch() const noexcept {
		return patch;
	}

	uint64_t toLongValue() const noexcept;

	int compareTo(const SoftwareVersion* other) const noexcept;

	static SoftwareVersion* parseString(const UnicodeString* str);

	// binary
	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static SoftwareVersion* createFromBinary(ByteBuffer* in);

protected:
	int major;
	int minor;
	int patch;
};

} /* namespace codablecash */

#endif /* BC_SOFTWAREVERSION_H_ */
