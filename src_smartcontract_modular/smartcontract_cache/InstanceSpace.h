/*
 * InstanceSpace.h
 *
 *  Created on: Nov 28, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_CACHE_INSTANCESPACE_H_
#define SMARTCONTRACT_CACHE_INSTANCESPACE_H_
#include "base/ArrayList.h"

namespace alinous {
class ByteBuffer;
class File;
class VirtualMachine;
class SysMutex;
class AbstractFunctionExtArguments;
}
using namespace alinous;


namespace codablecash {

class ModularSmartcontractInstance;
class SmartcontractInstanceAddress;
class SmartcontractExecResult;
class SmartcontractProjectId;
class CdbDatabaseSessionId;

class InstanceSpace {
public:
	InstanceSpace(const SmartcontractInstanceAddress *instAddress, ModularSmartcontractInstance* instance);
	virtual ~InstanceSpace();

	bool isDeletable() const noexcept;
	void beforeCacheOut();

	const SmartcontractInstanceAddress* getSmartContractInstanceAddress() const noexcept;

	// language
	bool analyze();
	void setMainInstance();

	// before invoke
	bool createMainInstance();
	bool interpretInitializer();
	void resetRootReference();

	// dependent modules
	void invokeModularProxyListnerMethod();

	// databases
	void setDatabaseDir(const File* baseDir);
	void createDatabase();

	void newSession(const CdbDatabaseSessionId *sessionId);
	const CdbDatabaseSessionId* getDatabaseSessionId() const noexcept;
	void undoCurrentSession();

	// exec
	void loadDatabase();
	void cleanDbRoot();

	// ref
	void incRef();
	void decRef();

	// invoke main instance
	SmartcontractExecResult* invokeMainObjectMethod(UnicodeString *moduleName, UnicodeString* methodName, ArrayList<AbstractFunctionExtArguments>* args);

	const SmartcontractProjectId* getProjectId() const noexcept {
		return this->projectId;
	}

private:
	ModularSmartcontractInstance* instance;

	SysMutex* mutex;
	int ref;

	// cached
	SmartcontractProjectId* projectId;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_CACHE_INSTANCESPACE_H_ */
