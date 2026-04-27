/*
 * ModularProxyListnerClassDeclare.h
 *
 *  Created on: Dec 3, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_INTERFACES_MODULARPROXYLISTNERCLASSDECLARE_H_
#define MODULAR_INTERFACES_MODULARPROXYLISTNERCLASSDECLARE_H_

#include "instance/reserved_classes/AbstractReservedClassDeclare.h"

using namespace alinous;

namespace codablecash {

class ModularProxyListnerClassDeclare : public AbstractReservedClassDeclare {
public:
	static const UnicodeString PACKAGE_NAME;
	static const UnicodeString NAME;

	static AnalyzedClass* createAnalyzedClass() noexcept;

	ModularProxyListnerClassDeclare();
	virtual ~ModularProxyListnerClassDeclare();

	virtual uint16_t getClassType() const noexcept {
		return TYPE_MODULAR_PROXY_LISTNER;
	}

	virtual const UnicodeString* getPackageName() const noexcept;
};

} /* namespace codablecash */

#endif /* MODULAR_INTERFACES_MODULARPROXYLISTNERCLASSDECLARE_H_ */
