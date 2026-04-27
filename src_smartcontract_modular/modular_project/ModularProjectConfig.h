/*
 * ModularProjectConfig.h
 *
 *  Created on: Oct 4, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_MODULARPROJECTCONFIG_H_
#define MODULAR_PROJECT_MODULARPROJECTCONFIG_H_

#include "base/ArrayList.h"

namespace alinous {
class UnicodeString;
class File;
class ByteBuffer;
}
using namespace alinous;

namespace codablecash {

class SoftwareVersion;

class ModularProjectConfig {
public:
	static constexpr const wchar_t* PROJECT{L"project"};
	static constexpr const wchar_t* VERSION = L"version";
	static constexpr const wchar_t* EXECUTABLE{L"executable"};
	static constexpr const wchar_t* LIBRALIES{L"libraries"};
	static constexpr const wchar_t* AUTHOR{L"author"};
	static constexpr const wchar_t* LICENSE{L"license"};

	ModularProjectConfig();
	virtual ~ModularProjectConfig();

	void load(const File* file);

	void setProjectName(const UnicodeString* name) noexcept;

	void setVersion(const SoftwareVersion* ver) noexcept;
	const SoftwareVersion* getVersion() const noexcept {
		return this->version;
	}

	void setExecutable(const UnicodeString* exec) noexcept;
	const UnicodeString* getExecutable() const noexcept {
		return this->executable;
	}

	void setAuthor(const UnicodeString* author) noexcept;
	void setLicense(const UnicodeString* license) noexcept;

	void addLibrary(const UnicodeString* libname) noexcept;
	const ArrayList<UnicodeString>* getLibralies() const noexcept {
		return this->libralies;
	}

	int binarySize() const;
	void toBinary(ByteBuffer* out) const;
	static ModularProjectConfig* createFromBinary(ByteBuffer* in);

	ModularProjectConfig* copy() const;

private:
	UnicodeString* projectName;
	SoftwareVersion* version;
	UnicodeString* executable;

	ArrayList<UnicodeString>* libralies;

	UnicodeString* author;
	UnicodeString* license;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_MODULARPROJECTCONFIG_H_ */
