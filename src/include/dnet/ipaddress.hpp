#ifndef INCLUDE_DNET_IPADDRESS
#define INCLUDE_DNET_IPADDRESS

#include <string>
#include <iostream>

namespace dnet {

	class IPAddress {
	public:
		IPAddress();
		IPAddress(const std::string&, const unsigned int p);
		IPAddress(const unsigned int ip, const unsigned int p);
		IPAddress(const IPAddress&);
		IPAddress& operator=(const IPAddress&);

		void setIP(const std::string);
		void setDecimalIP(const unsigned int);

		void setPort(const unsigned int);

		const std::string getIP() const {
			return ip;
		}

		const unsigned int getPort() const {
			return port;
		}
	protected:
	private:
		std::string	ip;
		unsigned int port;
	};

}; // namespace dnet

// TODO: I wonder if the most correct is to place this within the namespace?
std::ostream& operator<< (std::ostream&, const dnet::IPAddress&);

#endif // INCLUDE_DNET_IPADDRESS
