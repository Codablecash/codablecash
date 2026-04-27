/*
 * ProjectIdKeyFactory.h
 *
 *  Created on: Nov 27, 2025
 *      Author: iizuka
 */

#ifndef MODULAR_PROJECT_REGISTORY_PROJECTIDKEYFACTORY_H_
#define MODULAR_PROJECT_REGISTORY_PROJECTIDKEYFACTORY_H_

#include "btreekey/BtreeKeyFactory.h"

using namespace alinous;

namespace codablecash {

class ProjectIdKeyFactory : public BtreeKeyFactory {
public:
	static const constexpr uint32_t PROJECT_ID_KEY{0x24};

	ProjectIdKeyFactory();
	virtual ~ProjectIdKeyFactory();

	virtual AbstractBtreeKey* fromBinary(uint32_t keyType, ByteBuffer* in) const;
	virtual BtreeKeyFactory* copy() const noexcept;
};

} /* namespace codablecash */

#endif /* MODULAR_PROJECT_REGISTORY_PROJECTIDKEYFACTORY_H_ */
