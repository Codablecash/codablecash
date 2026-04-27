/*
 * InstanceDependencyHandler.h
 *
 *  Created on: Nov 21, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_INSTANCE_INSTANCEDEPENDENCYHANDLER_H_
#define SMARTCONTRACT_INSTANCE_INSTANCEDEPENDENCYHANDLER_H_

#include "base/HashMap.h"

namespace alinous {
class UnicodeString;
class AnalyzeContext;
}
using namespace alinous;


namespace codablecash {

class AbstractExecutableModuleInstance;
class AbstractDependencyConfig;
class InstanceDependencyContext;
class ModularSmartcontractInstance;

class InstanceDependencyHandler {
public:
	InstanceDependencyHandler();
	virtual ~InstanceDependencyHandler();

	void registerDependentInstance(AbstractDependencyConfig* config, ModularSmartcontractInstance* parent, AnalyzeContext* actx);
	void importExportedClasses(AnalyzeContext* actx);

	void preAnalyze(AnalyzeContext* actx);
	void analyzeType(AnalyzeContext* actx);
	void analyze(AnalyzeContext* actx);

	ArrayList<InstanceDependencyContext>* getContextList() const noexcept {
		return this->list;
	}

private:
	HashMap<UnicodeString, InstanceDependencyContext>* map;
	ArrayList<InstanceDependencyContext>* list;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_INSTANCE_INSTANCEDEPENDENCYHANDLER_H_ */
