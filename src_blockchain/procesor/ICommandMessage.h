/*
 * ICommandMessage.h
 *
 *  Created on: 2022/03/07
 *      Author: iizuka
 */

#ifndef PROCESOR_ICOMMANDMESSAGE_H_
#define PROCESOR_ICOMMANDMESSAGE_H_

#include "procesor/ICommandParameter.h"

namespace codablecash {

class ICommandParameter;

class ICommandMessage : public ICommandParameter {
public:
	ICommandMessage();
	virtual ~ICommandMessage();

	virtual void execute(ICommandParameter* param) = 0;
};

} /* namespace codablecash */

#endif /* PROCESOR_ICOMMANDMESSAGE_H_ */
