/*
 * MolularSmartcontractCacheManager.cpp
 *
 *  Created on: Sep 22, 2025
 *      Author: iizuka
 */

#include "smartcontract_cache/MolularSmartcontractCacheManager.h"
#include "smartcontract_cache/InstanceSpace.h"

#include "base_thread/SysMutex.h"
#include "base_thread/StackUnlocker.h"

#include "transaction/SmartcontractInstanceAddress.h"


namespace codablecash {

MolularSmartcontractCacheManager::MolularSmartcontractCacheManager(int cacheLimit) {
	this->cacheLimit = cacheLimit;
	this->list = new RawLinkedList<InstanceSpace>();
	this->mutex = new SysMutex();
}

MolularSmartcontractCacheManager::~MolularSmartcontractCacheManager() {
	removeAll();

	delete this->list;
	delete this->mutex;
}

void MolularSmartcontractCacheManager::registerInstance(InstanceSpace *instSpace) {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	const SmartcontractInstanceAddress* address = instSpace->getSmartContractInstanceAddress();
	InstanceSpace* inst = __getInstanceSpace(address);
	assert(inst == nullptr);

	this->list->add(0, instSpace);

	__removeLast();
}

InstanceSpace* MolularSmartcontractCacheManager::getInstanceSpace(const SmartcontractInstanceAddress *address) {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	InstanceSpace* inst = __getInstanceSpace(address);
	if(inst != nullptr){
		int index = this->list->indexOf(inst);
		this->list->remove(index);
		this->list->add(inst);

		inst->incRef();
	}

	return inst;
}

InstanceSpace* MolularSmartcontractCacheManager::__getInstanceSpace(const SmartcontractInstanceAddress *address) const noexcept {
	InstanceSpace* instance = nullptr;

	int maxLoop = this->list->size();
	for(int i = 0; i != maxLoop; ++i){
		InstanceSpace* inst = this->list->get(i);
		const SmartcontractInstanceAddress* ad = inst->getSmartContractInstanceAddress();

		if(address->compareTo(ad) == 0){
			instance = inst;
			break;
		}
	}

	return instance;
}

void MolularSmartcontractCacheManager::__removeLast() {
	int size = this->list->size();
	if(size > this->cacheLimit){
		int pos = 0;
		while(pos < size){
			InstanceSpace* inst = this->list->get(pos);

			if(inst->isDeletable()){
				inst->beforeCacheOut();
				this->list->remove(pos);
				delete inst;
				break;
			}
			pos++;
		}
	}
}

void MolularSmartcontractCacheManager::removeAll() {
	int size = cacheSize();

	while(size > 0){
		{
			StackUnlocker __lock(this->mutex, __FILE__, __LINE__);
			this->cacheLimit = 0;
			__removeLast();
		}
		size = cacheSize();
	}
}

int MolularSmartcontractCacheManager::cacheSize() const noexcept {
	StackUnlocker __lock(this->mutex, __FILE__, __LINE__);

	return this->list->size();
}

} /* namespace codablecash */
