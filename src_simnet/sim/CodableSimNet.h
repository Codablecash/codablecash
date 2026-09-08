/*
 * CodableSimNet.h
 *
 *  Created on: Sep 2, 2026
 *      Author: iizuka
 */

#ifndef SIM_CODABLESIMNET_H_
#define SIM_CODABLESIMNET_H_

#include "base/ArrayList.h"

using namespace alinous;

namespace codablecash {

class ZoneSimulator;
class JsonObject;

class CodableSimNet {
public:
	CodableSimNet();
	virtual ~CodableSimNet();

	JsonObject* makeJsonSettingObject();

private:
	ArrayList<ZoneSimulator>* zones;
};

} /* namespace codablecash */

#endif /* SIM_CODABLESIMNET_H_ */
