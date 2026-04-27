/*
 * IpV6ClientConnection.h
 *
 *  Created on: 2022/02/10
 *      Author: iizuka
 */

#ifndef IPCONNECT_IPV6CLIENTCONNECTION_H_
#define IPCONNECT_IPV6CLIENTCONNECTION_H_

#include "ipconnect/IClientSocket.h"

#include "osenv/sockets.h"

#include "bc_p2p_info/P2pNodeRecord.h"

namespace codablecash {

class ClientConnectionSourceDescriptor;

class IpV6ClientConnection : public IClientSocket {
public:
	IpV6ClientConnection();
	explicit IpV6ClientConnection(SOCKET_ID sock);
	virtual ~IpV6ClientConnection();

	virtual IClientSocket* newSameClient();

	virtual void connect(const UnicodeString* host, int port);
	virtual void close();

	virtual int read(char* buff, int len);
	virtual int write(const char* buff, int len);

	virtual int readPool(int sec, int usec);

	virtual bool isConnected();

	virtual void setClientAddress(const ClientConnectionSourceDescriptor* desc) noexcept;

	virtual int getSocketType() {
		return P2pNodeRecord::TCP_IP_V6;
	}

private:
	SOCKET_ID sock;
	ClientConnectionSourceDescriptor* client;
	int port;
};

} /* namespace codablecash */

#endif /* IPCONNECT_IPV6CLIENTCONNECTION_H_ */
