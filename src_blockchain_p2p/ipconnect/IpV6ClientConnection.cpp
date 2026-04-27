/*
 * IpV6ClientConnection.cpp
 *
 *  Created on: 2022/02/10
 *      Author: iizuka
 */

#include "ipconnect/IpV6ClientConnection.h"
#include "ipconnect/ClientConnectionSourceDescriptor.h"

#include "base/UnicodeString.h"


namespace codablecash {

IpV6ClientConnection::IpV6ClientConnection() {
	this->sock = 0;
	this->client = nullptr;
	this->port = -1;
}

IpV6ClientConnection::IpV6ClientConnection(int sock) {
	this->sock = sock;
	this->client = nullptr;
	this->port = -1;
}

IpV6ClientConnection::~IpV6ClientConnection() {
	close();
}

IClientSocket* IpV6ClientConnection::newSameClient() {
	return new IpV6ClientConnection();
}

void IpV6ClientConnection::connect(const UnicodeString* host, int port) {
	UnicodeString strPort(L"");
	strPort.append((int64_t)port);

	this->sock = IPV6::connect(host, &strPort);
	this->port = port;
}

void IpV6ClientConnection::close() {
	if(isConnected()){
		IPV6::close(this->sock);
		this->sock = 0;

		delete this->client;
		this->client = nullptr;
	}
}

int IpV6ClientConnection::readPool(int sec, int usec) {
	return IPV6::readPool(this->sock, sec, usec);
}

bool codablecash::IpV6ClientConnection::isConnected() {
	return this->sock > 0 && !IPV6::hasException(this->sock, 0, 1);
}

int IpV6ClientConnection::read(char* buff, int len) {
	return IPV6::read(this->sock, buff, len);
}

int IpV6ClientConnection::write(const char* buff, int len) {
	return IPV6::write(this->sock, buff, len);
}

void IpV6ClientConnection::setClientAddress(const ClientConnectionSourceDescriptor *desc) noexcept {
	delete client;
	this->client = new ClientConnectionSourceDescriptor(*desc);
}

} /* namespace codablecash */
