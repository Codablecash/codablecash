/*
 * MoveTestUtil.h
 *
 *  Created on: Mar 14, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_MODULAR_INSTANCE_MOVE_MOVETESTUTIL_H_
#define SMARTCONTRACT_MODULAR_INSTANCE_MOVE_MOVETESTUTIL_H_
#include <cstdint>

#include "base/ArrayList.h"
namespace alinous {
class File;
class AbstractFunctionExtArguments;
}
using namespace alinous;


namespace codablecash {

class ModularSmartcontractProject;
class SmartcontractModuleAddress;
class ModularSmartcontractExecutor;
class BlockHeaderId;
class SmartcontractInstanceAddress;
class InstanceSessionContext;
class CdbDatabaseSessionId;


class MoveTestUtil {
public:
	MoveTestUtil(ModularSmartcontractProject* project, File* baseTestDir);
	virtual ~MoveTestUtil();

	void init();
	void registerProject();
	void registerProject(uint64_t regHeight, BlockHeaderId* regHeaderId);

	void createInstance(const SmartcontractInstanceAddress* instAddress, uint64_t creHeight);
	void createInstance(const SmartcontractInstanceAddress* instAddress, BlockHeaderId* blockHeaderId, uint64_t height);
	void process(const SmartcontractInstanceAddress* address, const UnicodeString* module, const UnicodeString* methodName, ArrayList<AbstractFunctionExtArguments>* args
			, BlockHeaderId *blockHeaderId, uint64_t height);
	CdbDatabaseSessionId* process(const SmartcontractInstanceAddress* address, const UnicodeString* module, const UnicodeString* methodName, ArrayList<AbstractFunctionExtArguments>* args
			, BlockHeaderId *blockHeaderId, uint64_t height, const CdbDatabaseSessionId* lastTrxId);

	ModularSmartcontractExecutor* getExecutor() const noexcept {
		return this->executor;
	}

	ArrayList<InstanceSessionContext>* getHeads(const SmartcontractInstanceAddress* address) const;

private:
	ModularSmartcontractProject* project;
	File* baseTestDir;

	SmartcontractModuleAddress* moduleAddress;

	ModularSmartcontractExecutor* executor;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_MODULAR_INSTANCE_MOVE_MOVETESTUTIL_H_ */
