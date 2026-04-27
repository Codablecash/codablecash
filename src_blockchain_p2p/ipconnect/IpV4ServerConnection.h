/*
 * IpV4ServerConnection.h
 *
 *  Created on: Apr 18, 2026
 *      Author: iizuka
 */

#ifndef IPCONNECT_IPV4SERVERCONNECTION_H_
#define IPCONNECT_IPV4SERVERCONNECTION_H_

#include "osenv/sockets.h"

#include "ipconnect/IServerSocket.h"

#include "bc_p2p_info/P2pNodeRecord.h"

namespace alinous {
class UnicodeString;
}
using namespace alinous;

namespace codablecash {

class IpV4ServerConnection : public IServerSocket {
public:
	IpV4ServerConnection();
	virtual ~IpV4ServerConnection();

	virtual void initAddress(const UnicodeString* host, int port);
	virtual void bind();
	virtual void listen();

	virtual int poolAccept(int sec, int usec);
	virtual IClientSocket* accept();

	virtual void close();

	static void checkInetFamily(int family);

	virtual int getSocketType() {
		return P2pNodeRecord::TCP_IP_V4;
	}

private:
	IPV6Context context;
};

} /* namespace codablecash */

#endif /* IPCONNECT_IPV4SERVERCONNECTION_H_ */
