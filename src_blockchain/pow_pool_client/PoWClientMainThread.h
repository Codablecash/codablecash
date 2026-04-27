/*
 * PoWClientMainThread.h
 *
 *  Created on: Apr 15, 2026
 *      Author: iizuka
 */

#ifndef POW_POOL_CLIENT_POWCLIENTMAINTHREAD_H_
#define POW_POOL_CLIENT_POWCLIENTMAINTHREAD_H_

#include "base_thread/AbstractThreadRunner.h"

namespace alinous {
class UnicodeString;
class SynchronizedLock;
}
using namespace alinous;

namespace codablecash {

class ISystemLogger;
class PoWWorkerClient;
class PoWRequest2Client;


class PoWClientMainThread : public alinous::AbstractThreadRunner {
public:
	PoWClientMainThread(PoWWorkerClient* powClient, ISystemLogger* logger, const UnicodeString* name);
	virtual ~PoWClientMainThread();

	virtual void process() noexcept;

	void setRunning(bool bl) noexcept;
	bool isRunning() const noexcept;

private:
	void mainProcess();

	void calculate(PoWRequest2Client* request);
	bool getMainStatus();
	PoWRequest2Client* getCalculationRequest();

private:
	PoWWorkerClient* powClient;
	ISystemLogger* logger;

	SynchronizedLock* lock;
	volatile bool running;

	int loopCount;
};

} /* namespace codablecash */

#endif /* POW_POOL_CLIENT_POWCLIENTMAINTHREAD_H_ */
