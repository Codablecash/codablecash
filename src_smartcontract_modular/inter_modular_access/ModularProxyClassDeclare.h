/*
 * ModularProxyClassDeclare.h
 *
 *  Created on: Dec 3, 2025
 *      Author: iizuka
 */

#ifndef INTER_MODULAR_ACCESS_MODULARPROXYCLASSDECLARE_H_
#define INTER_MODULAR_ACCESS_MODULARPROXYCLASSDECLARE_H_

#include "instance/reserved_classes/AbstractReservedClassDeclare.h"

using namespace alinous;

namespace codablecash {

class InstanceDependencyContext;

class ModularProxyClassDeclare : public AbstractReservedClassDeclare {
public:
	ModularProxyClassDeclare();
	virtual ~ModularProxyClassDeclare();

//	static AnalyzedClass* createAnalyzedClass() noexcept;

	virtual uint16_t getClassType() const noexcept {
		return AbstractReservedClassDeclare::TYPE_MODULAR_PROXY;
	}

	//// use getName() and GetPackageName of ClassDeclare
	///  use Extends
	static const UnicodeString* getPackageFromIf(ClassDeclare *ifdec);
	static UnicodeString* getNameFromInterface(const UnicodeString* mainFqn, const UnicodeString* ifName);

	void setFactory(IVmInstanceFactory* factory);
	virtual IVmInstanceFactory* getFactory() const noexcept;

	void setDependencyContext(InstanceDependencyContext* dctx) noexcept;
	InstanceDependencyContext* getInstanceDependencyContext() const noexcept {
		return this->dctx;
	}

	virtual void preAnalyze(AnalyzeContext* actx);

private:
	IVmInstanceFactory* factory;
	InstanceDependencyContext* dctx;
};

} /* namespace codablecash */

#endif /* INTER_MODULAR_ACCESS_MODULARPROXYCLASSDECLARE_H_ */
