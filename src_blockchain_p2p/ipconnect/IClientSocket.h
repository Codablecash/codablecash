/*
 * IClientSocket.h
 *
 *  Created on: 2022/02/11
 *      Author: iizuka
 */

#ifndef IPCONNECT_ICLIENTSOCKET_H_
#define IPCONNECT_ICLIENTSOCKET_H_

namespace alinous {
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class ClientConnectionSourceDescriptor;

class IClientSocket {
public:
	IClientSocket();
	virtual ~IClientSocket();

	//virtual IClientSocket* newSameClient() = 0;

	virtual void connect(const UnicodeString* host, int port) = 0;
	virtual void close() = 0;
	virtual void setClientAddress(const ClientConnectionSourceDescriptor* desc) noexcept = 0;

	virtual int read(char* buff, int len) = 0;
	virtual int write(const char* buff, int len) = 0;

	virtual int readPool(int sec, int usec) = 0;

	virtual bool isConnected() = 0;

	virtual int getSocketType() = 0;
};

} /* namespace codablecash */

#endif /* IPCONNECT_ICLIENTSOCKET_H_ */
