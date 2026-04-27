/*
 * IpV4ClientConnection.cpp
 *
 *  Created on: 2023/09/09
 *      Author: iizuka
 */

#include "ipconnect/IpV4ClientConnection.h"
#include "ipconnect/ClientConnectionSourceDescriptor.h"

#include "base/UnicodeString.h"

namespace codablecash {

IpV4ClientConnection::IpV4ClientConnection() {
	this->sock = 0;
	this->client = nullptr;
}

IpV4ClientConnection::IpV4ClientConnection(SOCKET_ID sock) {
	this->sock = sock;
	this->client = nullptr;
}

IpV4ClientConnection::~IpV4ClientConnection() {
	close();
}

void IpV4ClientConnection::connect(const UnicodeString *host, int port) {
	UnicodeString strPort(L"");
	strPort.append((int64_t)port);

	this->sock = IPV4::connect(host, &strPort);
}

void IpV4ClientConnection::close() {
	if(isConnected()){
		IPV6::close(this->sock);
		this->sock = 0;

		delete this->client;
		this->client = nullptr;
	}
}

int IpV4ClientConnection::read(char *buff, int len) {
	return IPV6::read(this->sock, buff, len);
}

int IpV4ClientConnection::write(const char *buff, int len) {
	return IPV6::write(this->sock, buff, len);
}

int IpV4ClientConnection::readPool(int sec, int usec) {
	return IPV6::readPool(this->sock, sec, usec);
}

bool IpV4ClientConnection::isConnected() {
	return this->sock > 0 && !IPV6::hasException(this->sock, 0, 1);
}

void IpV4ClientConnection::setClientAddress(const ClientConnectionSourceDescriptor *desc) noexcept {
	delete client;
	this->client = new ClientConnectionSourceDescriptor(*desc);
}

} /* namespace codablecash */
