/*
 * MolularSmartcontractCacheManager.h
 *
 *  Created on: Sep 22, 2025
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_CACHE_MOLULARSMARTCONTRACTCACHEMANAGER_H_
#define SMARTCONTRACT_CACHE_MOLULARSMARTCONTRACTCACHEMANAGER_H_

#include "base/RawLinkedList.h"

namespace alinous {
class SysMutex;
}
using namespace alinous;

namespace codablecash {

class InstanceSpace;
class SmartcontractInstanceAddress;

class MolularSmartcontractCacheManager {
public:
	explicit MolularSmartcontractCacheManager(int cacheLimit);
	virtual ~MolularSmartcontractCacheManager();

	void registerInstance(InstanceSpace* instSpace);

	InstanceSpace* getInstanceSpace(const SmartcontractInstanceAddress* address);

	void removeAll();
	int cacheSize() const noexcept;

private:
	InstanceSpace* __getInstanceSpace(const SmartcontractInstanceAddress* address) const noexcept;
	void __removeLast();

private:
	RawLinkedList<InstanceSpace>* list;
	int cacheLimit;
	SysMutex* mutex;

};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_CACHE_MOLULARSMARTCONTRACTCACHEMANAGER_H_ */
