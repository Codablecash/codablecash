/*
 * IpV4ClientConnection.h
 *
 *  Created on: 2023/09/09
 *      Author: iizuka
 */

#ifndef IPCONNECT_IPV4CLIENTCONNECTION_H_
#define IPCONNECT_IPV4CLIENTCONNECTION_H_

#include "ipconnect/IClientSocket.h"

#include "osenv/sockets.h"

#include "bc_p2p_info/P2pNodeRecord.h"

namespace codablecash {

class IpV4ClientConnection : public IClientSocket {
public:
	IpV4ClientConnection();
	explicit IpV4ClientConnection(SOCKET_ID sock);
	virtual ~IpV4ClientConnection();

	virtual void connect(const UnicodeString* host, int port);
	virtual void close();
	virtual void setClientAddress(const ClientConnectionSourceDescriptor* desc) noexcept;

	virtual int read(char* buff, int len);
	virtual int write(const char* buff, int len);

	virtual int readPool(int sec, int usec);

	virtual bool isConnected();

	virtual int getSocketType() {
		return P2pNodeRecord::TCP_IP_V4;
	}

private:
	SOCKET_ID sock;
	ClientConnectionSourceDescriptor* client;
};

} /* namespace codablecash */

#endif /* IPCONNECT_IPV4CLIENTCONNECTION_H_ */
