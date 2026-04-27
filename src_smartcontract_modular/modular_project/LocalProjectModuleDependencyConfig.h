/*
 * LocalProjectModuleDependencyConfig.h
 *
 *  Created on: Nov 14, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_LOCALPROJECTMODULEDEPENDENCYCONFIG_H_
#define MODULAR_PROJECT_LOCALPROJECTMODULEDEPENDENCYCONFIG_H_

#include "modular_project/AbstractDependencyConfig.h"

namespace codablecash {

class LocalProjectModuleDependencyConfig;
class JsonObject;

class LocalProjectModuleDependencyConfig : public AbstractDependencyConfig {
public:
	static constexpr const wchar_t* LOCALLOC = L"localloc";

	LocalProjectModuleDependencyConfig(const LocalProjectModuleDependencyConfig& inst);
	LocalProjectModuleDependencyConfig();
	virtual ~LocalProjectModuleDependencyConfig();

	virtual AbstractDependencyConfig* copy() const;

	static LocalProjectModuleDependencyConfig* fromJsonObject(const JsonObject* jsonObj);

	void setLocalLoc(const UnicodeString* lloc) noexcept;

	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

private:
	UnicodeString* localloc;

};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_LOCALPROJECTMODULEDEPENDENCYCONFIG_H_ */
