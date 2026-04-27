/*
 * IServerSocket.h
 *
 *  Created on: 2022/02/11
 *      Author: iizuka
 */

#ifndef IPCONNECT_ISERVERSOCKET_H_
#define IPCONNECT_ISERVERSOCKET_H_

namespace alinous {
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class IClientSocket;

class IServerSocket {
public:

	IServerSocket();
	virtual ~IServerSocket();

	virtual void initAddress(const UnicodeString* host, int port) = 0;
	virtual void bind() = 0;
	virtual void listen() = 0;

	virtual int poolAccept(int sec, int usec) = 0;
	virtual IClientSocket* accept() = 0;

	virtual void close() = 0;

	virtual int getSocketType() = 0;
};

} /* namespace codablecash */

#endif /* IPCONNECT_ISERVERSOCKET_H_ */
