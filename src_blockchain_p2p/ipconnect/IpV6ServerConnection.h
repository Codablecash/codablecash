/*
 * IpV6ServerConnection.h
 *
 *  Created on: 2022/02/10
 *      Author: iizuka
 */

#ifndef IPCONNECT_IPV6SERVERCONNECTION_H_
#define IPCONNECT_IPV6SERVERCONNECTION_H_

#include "ipconnect/IServerSocket.h"

#include "osenv/sockets.h"

#include "bc_p2p_info/P2pNodeRecord.h"

namespace alinous {
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class IClientSocket;

class IpV6ServerConnection : public IServerSocket {
public:
	IpV6ServerConnection();
	virtual ~IpV6ServerConnection();

	virtual void initAddress(const UnicodeString* host, int port);
	virtual void bind();
	virtual void listen();

	virtual int poolAccept(int sec, int usec);
	virtual IClientSocket* accept();

	virtual void close();

	static void checkInetFamily(int family);

	virtual int getSocketType() {
		return P2pNodeRecord::TCP_IP_V6;
	}

private:
	IPV6Context context;
};

} /* namespace codablecash */

#endif /* IPCONNECT_IPV6SERVERCONNECTION_H_ */
