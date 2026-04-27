/*
 * ModularInstance.h
 *
 *  Created on: Oct 8, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_MODULARINSTANCECONFIG_H_
#define MODULAR_PROJECT_MODULARINSTANCECONFIG_H_

#include "base/ArrayList.h"

namespace alinous {
class UnicodeString;
class ByteBuffer;
}
using namespace alinous;

namespace codablecash {

class AbstractJsonValue;
class JsonObject;
class JsonArrayObject;

class ModularInstanceConfig {
public:
	static const constexpr wchar_t* MAIN_PACKAGE = L"mainPackage";
	static const constexpr wchar_t* MAIN_CLASS = L"mainClass";
	static const constexpr wchar_t* INITIALIZER = L"initializer";
	static const constexpr wchar_t* METHOD = L"method";
	static const constexpr wchar_t* ARGS = L"args";
	static const constexpr wchar_t* LIB_EXPORT = L"libExport";
	static const constexpr wchar_t* DIRECT_ACCESS = L"directAccess";

	ModularInstanceConfig();
	virtual ~ModularInstanceConfig();

	void load(const JsonObject* instance);

	void setMainPackage(const UnicodeString* value) noexcept;
	void setMainClass(const UnicodeString* value) noexcept;
	void setInitializerMethod(const UnicodeString* value) noexcept;

	const UnicodeString* getMainPackage() const noexcept {
		return this->mainPackage;
	}
	const UnicodeString* getMainClass() const noexcept {
		return this->mainClass;
	}
	const UnicodeString* getInitializerMethod() const noexcept {
		return this->initializerMethod;
	}
	const ArrayList<AbstractJsonValue>* getInitializerMethodArguments() const noexcept {
		return this->initializerMethodArguments;
	}

	int binarySize() const;
	void toBinary(ByteBuffer* out) const;
	void fromBinary(ByteBuffer* in);
	static ModularInstanceConfig* createFromBinary(ByteBuffer* in);

	ArrayList<UnicodeString>* getLibExport() const noexcept {
		return this->libExport;
	}

	ArrayList<UnicodeString>* getDirectAccess() const noexcept {
		return this->directAccess;
	}

private:
	void loadMainPackageAndClass(const JsonObject* instance);
	void loadInitializer(const JsonObject* initializer);
	void loadLibExport(const JsonArrayObject* libExport);
	void loadDirectAccess(const JsonArrayObject* directAccess);

private:
	UnicodeString* mainPackage;
	UnicodeString* mainClass;
	UnicodeString* initializerMethod;

	ArrayList<AbstractJsonValue>* initializerMethodArguments;

	ArrayList<UnicodeString>* libExport;
	ArrayList<UnicodeString>* directAccess;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_MODULARINSTANCECONFIG_H_ */
