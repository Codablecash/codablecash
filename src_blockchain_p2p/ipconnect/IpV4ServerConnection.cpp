/*
 * IpV4ServerConnection.cpp
 *
 *  Created on: Apr 18, 2026
 *      Author: iizuka
 */

#include "ipconnect/IpV4ServerConnection.h"

#include "base/UnicodeString.h"

#include "base/StackRelease.h"

#include "ipconnect/ClientConnectionSourceDescriptor.h"

#include "ipconnect/IpV4ClientConnection.h"

#include "ipconnect/UnexpectedProtocolException.h"

#include "bc/ExceptionThrower.h"
namespace codablecash {

IpV4ServerConnection::IpV4ServerConnection() {

}

IpV4ServerConnection::~IpV4ServerConnection() {
	close();
}

void IpV4ServerConnection::initAddress(const UnicodeString *host, int port) {
	UnicodeString strPort(L"");
	strPort.append((int64_t)port);

	this->context = IPV4::socket(host, &strPort);
}

void IpV4ServerConnection::bind() {
	IPV6::bind(&this->context);
}

void IpV4ServerConnection::listen() {
	IPV6::listen(&this->context, 256);
}

int IpV4ServerConnection::poolAccept(int sec, int usec) {
	return IPV6::readPool(this->context.sockId, sec, usec);
}

IClientSocket* IpV4ServerConnection::accept() {
	IClientSocket* ret = nullptr;

	IPV6AcceptContext* accontext = IPV6::accept(&this->context); __STP(accontext);

	checkInetFamily(accontext->client.sin.sin_family);

	// ::ffff:127.0.0.1
	char buf[256];
	Mem::memset(buf, 0, sizeof(buf));
	const char* addrstr = IPV6::inet_ntop(AF_INET, &accontext->client.sin.sin_addr, buf, sizeof(buf));

	UnicodeString hostAddress(addrstr);
	int port = accontext->client.sin.sin_port;

	ClientConnectionSourceDescriptor desc(ClientConnectionSourceDescriptor::IPV4, &hostAddress, port);

	ret = new IpV4ClientConnection(accontext->sockId);
	ret->setClientAddress(&desc);

	return ret;
}

void IpV4ServerConnection::close() {
	IPV6::close(&this->context);
}

void IpV4ServerConnection::checkInetFamily(int family) {
	ExceptionThrower<UnexpectedProtocolException>::throwExceptionIfCondition(family != AF_INET, L"", __FILE__, __LINE__);
}

} /* namespace codablecash */
