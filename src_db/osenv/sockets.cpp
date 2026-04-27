/*
 * sockets.cpp
 *
 *  Created on: 2022/02/11
 *      Author: iizuka
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>



#include "osenv/sockets.h"

#include "base/StackRelease.h"
#include "base/UnicodeString.h"

namespace alinous {



IPV6Context::IPV6Context(const IPV6Context& inst) {
	this->info = inst.info;
	this->error = inst.error;
	this->sockId = inst.sockId;
}

IPV6Context::IPV6Context() {
	this->info = nullptr;
	this->error = 0;
	this->sockId = 0;
}

void IPV6Context::release() {
	if(this->info != nullptr){
		::freeaddrinfo(this->info);
		this->info = nullptr;
	}
}

IPV6Context IPV6::socket(const UnicodeString* hostname, const UnicodeString* port) {
	return socket(hostname, port, 0);
}

IPV6Context IPV6::socket(const UnicodeString* hostname, const UnicodeString* port, int protocol) {
	IPV6Context context;

	const char* host = hostname != nullptr ? hostname->toCString() : nullptr;
	StackArrayRelease<const char> hostrel(host);

	const char* portStr = port->toCString();
	StackArrayRelease<const char> portrel(portStr);

	struct addrinfo hints;
	::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	context.error = ::getaddrinfo(host, portStr, &hints, &context.info);
	if(context.error != 0){
		return context;
	}

	context.sockId = ::socket(context.info->ai_family, context.info->ai_socktype, protocol);
	if(context.sockId < 0){
		return context;
	}

	int yes = 1;
	::setsockopt(context.sockId,
	   SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

	return context;
}

int IPV6::bind(IPV6Context* context) {
	if (::bind(context->sockId, context->info->ai_addr, context->info->ai_addrlen) != 0) {
		return -1;
	}

	return 0;
}

int IPV6::listen(IPV6Context* context, int backlog) {
	int ret = ::listen(context->sockId, backlog);
	context->release();
	return ret;
}

IPV6AcceptContext* IPV6::accept(IPV6Context* context) {
	IPV6AcceptContext* cli = new IPV6AcceptContext(); __STP(cli);

	cli->len = sizeof(cli->client);
	cli->sockId = ::accept(context->sockId, (struct sockaddr *)&(cli->client), &cli->len);

	return __STP_MV(cli);
}

void IPV6::close(IPV6Context* context) {
	if(context->sockId > 0){
		IPV6::close(context->sockId);
		context->sockId = 0;
	}

	context->release();
}

void alinous::IPV6::close(SOCKET_ID sockId) {
	int ret = ::close(sockId);

	assert(ret == 0);
}

SOCKET_ID IPV6::connect(const UnicodeString* hostname, const UnicodeString* port) {
	const char* host = hostname != nullptr ? hostname->toCString() : nullptr;
	StackArrayRelease<const char> hostrel(host);

	const char* portStr = port->toCString();
	StackArrayRelease<const char> portrel(portStr);

	int err;
	struct addrinfo hints;
	struct addrinfo* res = nullptr;

	Mem::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;

	err = getaddrinfo(host, portStr, &hints, &res);
	if (err != 0){
		return -1;
	}

	SOCKET_ID sockfd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	err = ::connect(sockfd, res->ai_addr, res->ai_addrlen);
	if (err != 0) {
		close(sockfd);
		freeaddrinfo(res);
		return -1;
	}

	freeaddrinfo(res);

	return sockfd;
}

int IPV6::readPool(SOCKET_ID sockId, int sec, int usec) {
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockId, &readfds);

	struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;

    int ret = ::select(sockId + 1, &readfds, NULL, NULL, &tv);

    if(ret > 0 && FD_ISSET(sockId, &readfds)){
    	return ret;
    }

    return ret;
}

bool IPV6::hasException(int sockId, int sec, int usec) {
	fd_set exceptfds;
	FD_ZERO(&exceptfds);
	FD_SET(sockId, &exceptfds);

	struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;

    ::select(sockId + 1, NULL, NULL, &exceptfds, &tv);

    return FD_ISSET(sockId, &exceptfds);
}

int IPV6::read(SOCKET_ID sockId, char* buff, int len) {
	return ::recv(sockId, buff, len, 0);
}

int IPV6::write(SOCKET_ID sockId, const char* buff, int len) {
	int n = ::send(sockId, buff, len, 0);

	return n;
}

/*const char* IPV6::inet_ntoa(struct in_addr sin_addr) {
	return ::inet_ntoa(sin_addr);
}*/

const char* IPV6::inet_ntop(int __af, const void* __cp, char* __buf, socklen_t __len){
	return ::inet_ntop(__af, __cp, __buf, __len);
}

IPV6Context IPV4::socket(const UnicodeString* hostname, const UnicodeString* port) {
	return socket(hostname, port, 0);
}

IPV6Context IPV4::socket(const UnicodeString* hostname, const UnicodeString* port, int protocol) {
	IPV6Context context;

	const char* host = hostname != nullptr ? hostname->toCString() : nullptr;
	StackArrayRelease<const char> hostrel(host);

	const char* portStr = port->toCString();
	StackArrayRelease<const char> portrel(portStr);

	struct addrinfo hints;
	::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	context.error = ::getaddrinfo(host, portStr, &hints, &context.info);
	if(context.error != 0){
		return context;
	}

	context.sockId = ::socket(context.info->ai_family, context.info->ai_socktype, protocol);
	if(context.sockId < 0){
		return context;
	}

	int yes = 1;
	::setsockopt(context.sockId,
	   SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

	return context;
}

SOCKET_ID IPV4::connect(const UnicodeString *hostname, const UnicodeString *port) {
	const char* host = hostname != nullptr ? hostname->toCString() : nullptr;
	StackArrayRelease<const char> hostrel(host);

	const char* portStr = port->toCString();
	StackArrayRelease<const char> portrel(portStr);

	int err;
	struct addrinfo hints;
	struct addrinfo* res = nullptr;

	Mem::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;

	err = getaddrinfo(host, portStr, &hints, &res);
	if (err != 0){
		return -1;
	}

	SOCKET_ID sockfd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	err = ::connect(sockfd, res->ai_addr, res->ai_addrlen);
	if (err != 0) {
		close(sockfd);
		freeaddrinfo(res);
		return -1;
	}

	freeaddrinfo(res);

	return sockfd;
}

bool SocketPipeSigHandler::setSigPipeHander(SigPipeRoutine callback) noexcept {
    struct sigaction sa;
    sa.sa_handler = callback;
    ::sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    int ret = ::sigaction(SIGPIPE, &sa, NULL);
    return ret == 0;
}


#define PRINT_N(N) {\
	auto num = N; \
	char ch[10] = {0,0,0,0,0,0,0,0,0,0}; \
	char* p = &ch[9]; \
	while (num != 0) { \
		*p-- = '0' + num % 10; \
		num /= 10; \
	} \
	++p; \
	::write(1, p, sizeof(ch) - (p-&ch[0])); \
}

void SocketPipeSigHandler::defaultPipeHandler(int sig) {
	write(1, "interrupted: ", 13);
	PRINT_N(sig)
    write(1, "\n", 1);
    write(1, "thread id: ", 11);
    uint32_t tid = gettid();
    PRINT_N(tid);
    write(1, "\n", 1);
}

} /* namespace alinous */
