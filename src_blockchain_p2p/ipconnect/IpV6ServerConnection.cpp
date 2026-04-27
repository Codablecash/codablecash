/*
 * IpV6ServerConnection.cpp
 *
 *  Created on: 2022/02/10
 *      Author: iizuka
 */

#include "ipconnect/IpV6ServerConnection.h"
#include "ipconnect/IpV6ClientConnection.h"
#include "ipconnect/IpV4ClientConnection.h"
#include "ipconnect/UnexpectedProtocolException.h"

#include "base/UnicodeString.h"
#include "base/StackRelease.h"

#include "ipconnect/ClientConnectionSourceDescriptor.h"

#include "osenv/memory.h"

#include "sys/socket.h"

using alinous::Mem;

namespace codablecash {

IpV6ServerConnection::IpV6ServerConnection() {

}

IpV6ServerConnection::~IpV6ServerConnection() {
	close();
}

void IpV6ServerConnection::initAddress(const UnicodeString* host, int port) {
	UnicodeString strPort(L"");
	strPort.append((int64_t)port);

	this->context = IPV6::socket(host, &strPort);
}

void IpV6ServerConnection::bind() {
	IPV6::bind(&this->context);
}

void IpV6ServerConnection::listen() {
	IPV6::listen(&this->context, 256);
}

int IpV6ServerConnection::poolAccept(int sec, int usec) {
	return IPV6::readPool(this->context.sockId, sec, usec);
}


IClientSocket* IpV6ServerConnection::accept() {
	static UnicodeString IPV4PREFIX(L"::ffff:");

	IClientSocket* ret = nullptr;

	IPV6AcceptContext* accontext = IPV6::accept(&this->context); __STP(accontext);

	checkInetFamily(accontext->client.sin.sin_family);

	// ::ffff:127.0.0.1
	char buf[256];
	Mem::memset(buf, 0, sizeof(buf));
	const char* addrstr = IPV6::inet_ntop(AF_INET6, &accontext->client.sin6.sin6_addr, buf, sizeof(buf));

	UnicodeString hostAddress(addrstr);
	int port = accontext->client.sin6.sin6_port;

	if(hostAddress.startsWith(IPV4PREFIX)){ // IpV4
		UnicodeString* host = hostAddress.substring(IPV4PREFIX.length()); __STP(host);
		ClientConnectionSourceDescriptor desc(ClientConnectionSourceDescriptor::IPV4, host, port);

		ret = new IpV4ClientConnection(accontext->sockId);
		ret->setClientAddress(&desc);
	}
	else { // accontext->client.sin.sin_family == AF_INET6
		ClientConnectionSourceDescriptor desc(ClientConnectionSourceDescriptor::IPV6, &hostAddress, port);

		ret = new IpV6ClientConnection(accontext->sockId);
		ret->setClientAddress(&desc);
	}

	return ret;
}
void IpV6ServerConnection::close() {
	IPV6::close(&this->context);
}

void IpV6ServerConnection::checkInetFamily(int family) {
	if(family != AF_INET && family != AF_INET6){
		throw new UnexpectedProtocolException(__FILE__, __LINE__);
	}
}

} /* namespace codablecash */
