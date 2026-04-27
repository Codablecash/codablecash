/*
 * LibraryExectableModuleInstance.h
 *
 *  Created on: Nov 11, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_INSTANCE_LIBRARYEXECTABLEMODULEINSTANCE_H_
#define SMARTCONTRACT_INSTANCE_LIBRARYEXECTABLEMODULEINSTANCE_H_

#include "smartcontract_instance/AbstractExecutableModuleInstance.h"

namespace alinous {
class File;
}
using namespace alinous;

namespace codablecash {

class LibraryExectableModuleInstance : public AbstractExecutableModuleInstance {
public:
	LibraryExectableModuleInstance();
	virtual ~LibraryExectableModuleInstance();

	const UnicodeString* getLibraryName() const noexcept {
		return this->libraryName;
	}

	void setLibraryName(const UnicodeString* str) noexcept;
	void addExportClass(const UnicodeString* clazz) noexcept;

	virtual void loadCompilantUnits(const File* projectBaseDir);

	void loadExportClasses(ModuleInstanceClassLoader* loader);

	// binary
	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	virtual void fromBinary(ByteBuffer* in);

private:
	UnicodeString* libraryName;
	ArrayList<UnicodeString>* exportClasses;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_INSTANCE_LIBRARYEXECTABLEMODULEINSTANCE_H_ */
