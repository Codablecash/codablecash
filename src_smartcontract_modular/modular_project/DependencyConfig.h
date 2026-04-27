/*
 * DependencyConfig.h
 *
 *  Created on: Oct 8, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_DEPENDENCYCONFIG_H_
#define MODULAR_PROJECT_DEPENDENCYCONFIG_H_

#include "base/ArrayList.h"

namespace alinous {
class ByteBuffer;
}  // namespace alinous
using namespace alinous;


namespace codablecash {

class AbstractDependencyConfig;
class JsonArrayObject;

class DependencyConfig {
public:
	static constexpr const wchar_t* NAME = L"name";
	static constexpr const wchar_t* VERSION = L"version";

	DependencyConfig(const DependencyConfig& inst);
	DependencyConfig();
	virtual ~DependencyConfig();

	void load(const JsonArrayObject* dependencies);

	ArrayList<AbstractDependencyConfig>* getList() const noexcept {
		return this->list;
	}

	// binary
	virtual int binarySize() const;
	virtual void toBinary(ByteBuffer* out) const;
	static DependencyConfig* createFromBinary(ByteBuffer* in);

private:
	ArrayList<AbstractDependencyConfig>* list;

};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_DEPENDENCYCONFIG_H_ */
