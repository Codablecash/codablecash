/*
 * LibrarySmartcontractModule.h
 *
 *  Created on: Sep 19, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_LIBRARYSMARTCONTRACTMODULE_H_
#define MODULAR_PROJECT_LIBRARYSMARTCONTRACTMODULE_H_

#include "modular_project/AbstractSmartcontractModule.h"

namespace codablecash {

class LibrarySmartcontractModule : public AbstractSmartcontractModule {
public:
	static constexpr const wchar_t* LIBRARY_NAME = L"libraryName";
	static constexpr const wchar_t* EXPORT_CLASSES = L"exportClasses";

	explicit LibrarySmartcontractModule(const UnicodeString* projectRelativePath);
	virtual ~LibrarySmartcontractModule();

	virtual void load(const File* modulePath);

	void setLibraryName(const UnicodeString* str) noexcept;

	virtual AbstractExecutableModuleInstance* toInstance() const;

private:
	UnicodeString* libraryName;
	ArrayList<UnicodeString>* exportClasses;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_LIBRARYSMARTCONTRACTMODULE_H_ */
