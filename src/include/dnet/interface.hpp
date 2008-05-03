
#ifndef INCLUDE_DNET_INTERFACE_HPP
#define INCLUDE_DNET_INTERFACE_HPP

#include <stdint.h>

#include <sys/time.h>
#include <time.h>

namespace dnet {

class Packet;
class Buffer;

class Interface {
public:
	struct timeval startTime;
	
	Interface() {
		gettimeofday(&startTime, NULL);
	};
	
	virtual ~Interface() {};

	virtual uint8_t getId() {
		return 0;
	};

	virtual int send(Packet*, bool=false) = 0;
	virtual int send(Buffer&, bool=false ) = 0;
	virtual int sendReliable(Packet*) = 0;
	virtual int sendReliable(Buffer&) = 0;
};

} // namespace dnet

#endif // INCLUDE_DNET_INTERFACE_HPP
