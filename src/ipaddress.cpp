#include <dnet/ipaddress.hpp>
#include <sstream>

namespace dnet {

IPAddress::IPAddress(const std::string& i, const unsigned int p) : ip(i), port(p) {
}

IPAddress::IPAddress(const unsigned int i, const unsigned int p) : ip("127.0.0.1"), port(p) {

	this->setDecimalIP(i);
}

IPAddress::IPAddress(const IPAddress& other) : ip(other.ip), port(other.port)  {
}

IPAddress& IPAddress::operator=(const IPAddress& other) {
	this->setIP(other.ip);
	this->setPort(other.port);
	return *this;
}

void IPAddress::setIP(const std::string i) {
	this->ip = i;
}

void IPAddress::setDecimalIP(const unsigned int descip) {
	std::stringstream ips;

	ips << "" /*is necessary to avoid ambigious overload problem*/ <<
		int(descip&0xff) << "." << int((descip>>8)&0xff) << "." << 
		int((descip>>16)&0xff) <<  "." << int((descip>>24)&0xff);

	this->setIP(ips.str());
}

void IPAddress::setPort(const unsigned int p) {
	this->port = p;
}

} // namespace dnet

std::ostream& operator<<(std::ostream& os, const dnet::IPAddress& addr) {
	os << addr.getIP() << ":" << addr.getPort();
	return os;
}

