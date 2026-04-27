/*
 * sockets.h
 *
 *  Created on: 2022/02/11
 *      Author: iizuka
 */

#ifndef OSENV_SOCKETS_H_
#define OSENV_SOCKETS_H_

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace alinous {

class UnicodeString;

#define SOCKET_ID int

class IPV6Context {
public:
	IPV6Context(const IPV6Context& inst);
	IPV6Context();

	void release();

	SOCKET_ID sockId;
	struct addrinfo *info;
	int error;
};

class IPV6AcceptContext {
public:
	union {
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
	} client;

	socklen_t len;
	SOCKET_ID sockId;
};

class IPV6 {
public:
	static IPV6Context socket(const UnicodeString* hostname, const UnicodeString* port);
	static IPV6Context socket(const UnicodeString* hostname, const UnicodeString* port, int protocol);
	static int bind(IPV6Context* context);
	static int listen(IPV6Context* context, int backlog);
	static IPV6AcceptContext* accept(IPV6Context* context);
	static void close(IPV6Context* context);
	static void close(SOCKET_ID sockId);

	static SOCKET_ID connect(const UnicodeString* hostname, const UnicodeString* port);
	static int readPool(SOCKET_ID sockId, int sec, int usec);
	static bool hasException(SOCKET_ID sockId, int sec, int usec);
	static int read(SOCKET_ID sockId, char* buff, int len);
	static int write(SOCKET_ID sockId, const char* buff, int len);

//	static const char* inet_ntoa(struct in_addr sin_addr);
	static const char* inet_ntop(int __af, const void * __cp, char * __buf, socklen_t __len);
};

class IPV4 {
public:
	static IPV6Context socket(const UnicodeString* hostname, const UnicodeString* port);
	static IPV6Context socket(const UnicodeString* hostname, const UnicodeString* port, int protocol);
	static SOCKET_ID connect(const UnicodeString* hostname, const UnicodeString* port);
};

typedef void (*SigPipeRoutine) (int);

class SocketPipeSigHandler {
public:
	static bool setSigPipeHander(SigPipeRoutine callback) noexcept;

	static void defaultPipeHandler(int sig);

};

} /* namespace alinous */

#endif /* OSENV_SOCKETS_H_ */
