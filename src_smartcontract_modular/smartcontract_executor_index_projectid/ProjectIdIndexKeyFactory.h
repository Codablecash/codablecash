/*
 * ProjectIdIndexKeyFactory.h
 *
 *  Created on: Jan 29, 2026
 *      Author: iizuka
 */

#ifndef SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTIDINDEXKEYFACTORY_H_
#define SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTIDINDEXKEYFACTORY_H_

#include "btreekey/BtreeKeyFactory.h"

using namespace alinous;

namespace codablecash {

class ProjectIdIndexKeyFactory : public BtreeKeyFactory {
public:
	static const constexpr uint32_t PROJECT_ID_INDEX_KEY{0x24};
	static const constexpr uint32_t SMARTCONTRACT_VERSION_KEY{0x25};

	ProjectIdIndexKeyFactory();
	virtual ~ProjectIdIndexKeyFactory();

	virtual AbstractBtreeKey* fromBinary(uint32_t keyType, ByteBuffer* in) const;
	virtual BtreeKeyFactory* copy() const noexcept;
};

} /* namespace codablecash */

#endif /* SMARTCONTRACT_EXECUTOR_INDEX_PROJECTID_PROJECTIDINDEXKEYFACTORY_H_ */
