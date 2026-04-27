/*
 * ModularSmartcontractExecutor.h
 *
 *  Created on: Sep 22, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_MODULARSMARTCONTRACTEXECUTOR_H_
#define SMARTCONTRACT_EXECUTOR_MODULARSMARTCONTRACTEXECUTOR_H_

#include <cstdint>

#include "base/ArrayList.h"

namespace alinous {
class UnicodeString;
class File;
class AbstractFunctionExtArguments;
}
using namespace alinous;

namespace codablecash {

class ModularSmartcontractProjectRegistory;
class SmartcontractExecContextRegistory;
class InstanceIdIndex;
class ModularSmartcontractProject;
class SmartcontractProjectId;
class SmartcontractProjectData;
class InstanceSpacesManager;
class InstanceSpace;
class SmartcontractInstanceAddress;
class SmartcontractModuleAddress;
class RegisterSmartcontractProjectOperation;
class CreateSmartcontractInstanceOperation;
class ProcessSmartcontractOperation;
class CdbDatabaseSessionId;

class SmartcontractExecContextKey;
class SmartcontractExecContextData;
class ProjectIdIndex;

class BlockHeaderId;
class ProjectIdIndexKey;
class ProjectIndexData;

class InstanceIdIndexKey;
class InstanceIdIndexData;
class SmartcontractExecContextData;

class IInstanceSessionContextListner;


class ModularSmartcontractExecutor {
public:
	static const constexpr wchar_t* DIR_NAME{L"contract_exec"};
	static const constexpr wchar_t* INSTANCES_DIR_NAME{L"instances"};

	explicit ModularSmartcontractExecutor(const File* base);
	virtual ~ModularSmartcontractExecutor();

	void createExecutor();
	void open();
	void close() noexcept;

	void registerModularSmartcontractProject(ModularSmartcontractProject* project);
	void registerModularSmartcontractProject(SmartcontractProjectData* data);
	SmartcontractProjectData* getProject(const SmartcontractProjectId* projectId);

	// register index
	void registerAddress2ProjectIdIndex(const ProjectIdIndexKey *key, const ProjectIndexData *data);

	void createInstance(const SmartcontractInstanceAddress* instAddress, const SmartcontractProjectId* projectId);
	InstanceSpace* loadFromCache(const SmartcontractInstanceAddress* instAddress);

	// operations
	RegisterSmartcontractProjectOperation* makeRegisterSmartcontractProjectOperation(const SmartcontractModuleAddress *moduleAddress,
			SmartcontractProjectData* data, CdbDatabaseSessionId* trxId, uint64_t height, BlockHeaderId* blockHeaderId);
	CreateSmartcontractInstanceOperation* makeCreateSmartcontractInstanceOperation(const SmartcontractInstanceAddress* address, const SmartcontractProjectId* projectId,
			CdbDatabaseSessionId* trxId, uint64_t height, BlockHeaderId* blockHeaderId);
	ProcessSmartcontractOperation* makeProcessSmartcontractOperation(const SmartcontractInstanceAddress* address,
			const CdbDatabaseSessionId* sessionId, const CdbDatabaseSessionId* lastSessionId,
			const UnicodeString* module, const UnicodeString* methodName,
			ArrayList<AbstractFunctionExtArguments>* args,
			uint64_t height, const BlockHeaderId* blockHeaderId);


	// data
	void addExecContext(const SmartcontractExecContextKey* key, const SmartcontractExecContextData* data);
	SmartcontractExecContextData* getExecContext(const SmartcontractExecContextKey* key);

	void addInstanceIdIndex(const InstanceIdIndexKey* key, const InstanceIdIndexData* data);

	SmartcontractProjectId* getLatestProjectId(const SmartcontractModuleAddress* moduleAddress);

	SmartcontractExecContextRegistory* getSmartcontractExecContextRegistory() const noexcept {
		return this->execContextRegistory;
	}

	void findHeads(const SmartcontractInstanceAddress* address, IInstanceSessionContextListner* listner);

private:
	File* baseDir;
	ModularSmartcontractProjectRegistory* projectRegistory;
	ProjectIdIndex* address2ProjectIdIndex;

	SmartcontractExecContextRegistory* execContextRegistory;
	InstanceIdIndex* instanceIndex;

	InstanceSpacesManager* instanceSpace;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_MODULARSMARTCONTRACTEXECUTOR_H_ */
