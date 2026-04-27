/*
 * AbstractDependencyConfig.h
 *
 *  Created on: Oct 8, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_ABSTRACTDEPENDENCYCONFIG_H_
#define MODULAR_PROJECT_ABSTRACTDEPENDENCYCONFIG_H_
#include <cstdint>

namespace alinous {
class UnicodeString;
class ByteBuffer;
}
using namespace alinous;

namespace codablecash {

class ModularSmartcontractVersion;
class JsonObject;

class AbstractDependencyConfig {
public:
	static constexpr const wchar_t* NAME = L"name";
	static constexpr const wchar_t* VERSION = L"version";

	static constexpr const uint8_t TYPE_LOCAL_MODULE = 1;

	AbstractDependencyConfig(const AbstractDependencyConfig& inst);
	explicit AbstractDependencyConfig(uint8_t kind);
	virtual ~AbstractDependencyConfig();

	virtual AbstractDependencyConfig* copy() const = 0;

	static void fromJson(AbstractDependencyConfig* inst, const JsonObject* jsonObj);

	void setModuleName(const UnicodeString* name) noexcept;
	void setVersion(const ModularSmartcontractVersion* v) noexcept;

	const UnicodeString* getModuleName() const noexcept {
		return this->moduleName;
	}
	const ModularSmartcontractVersion* getVersion() const noexcept {
		return this->version;
	}

	bool isLocal() const noexcept {
		return true;
	}

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

	static AbstractDependencyConfig* createFromBinary(ByteBuffer* in);

private:
	uint8_t kind;
	UnicodeString* moduleName;
	ModularSmartcontractVersion* version;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_ABSTRACTDEPENDENCYCONFIG_H_ */
