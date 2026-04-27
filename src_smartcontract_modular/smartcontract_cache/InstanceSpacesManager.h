/*
 * InstanceSpacesManager.h
 *
 *  Created on: Nov 30, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_CACHE_INSTANCESPACESMANAGER_H_
#define SMARTCONTRACT_CACHE_INSTANCESPACESMANAGER_H_

namespace alinous {
class File;
}
using namespace alinous;

namespace codablecash {

class MolularSmartcontractCacheManager;
class InstanceSpace;
class SmartcontractExecContextRegistory;
class InstanceIdIndex;
class SmartcontractInstanceAddress;
class SmartcontractProjectId;
class ModularSmartcontractProjectRegistory;

class InstanceSpacesManager {
public:
	InstanceSpacesManager(const File* baseDir, SmartcontractExecContextRegistory* execContextReg
			, InstanceIdIndex* instanceIdIndex, ModularSmartcontractProjectRegistory* projectReg);
	virtual ~InstanceSpacesManager();

	InstanceSpace* createInstance(const SmartcontractInstanceAddress* instAddress, const SmartcontractProjectId* projectId);
	void registerCache(InstanceSpace* space);

	InstanceSpace* loadFromCache(const SmartcontractInstanceAddress *instAddress);

private:
	MolularSmartcontractCacheManager* cache;

	SmartcontractExecContextRegistory* execContextReg;
	InstanceIdIndex* instanceIdIndex;
	ModularSmartcontractProjectRegistory* projectReg;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_CACHE_INSTANCESPACESMANAGER_H_ */
