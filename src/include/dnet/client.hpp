#ifndef INCLUDE_DNET_CLIENT_HPP
#define INCLUDE_DNET_CLIENT_HPP

#include<string>
#include<enet/enet.h>
#include<dnet/ipaddress.hpp>
#include<dnet/interface.hpp>

namespace dnet {

class Packet;
class Buffer;

class Client : public Interface {
private:
	ENetAddress	address;
	ENetHost*	client;		// FIXME
	ENetPeer*	peer;		// FIXME
	uint8_t		id;

	void rawDisconnect();

protected:
	enum state_t {
		S_DISCONNECTED,		// No connection
		S_CONNECTING,		// Connecting to server, awaiting GreetingPacket
		S_CONNECTED			// Fully Connected
	};
	state_t state;

public:
	Client();
	virtual ~Client();

	bool connect(const char* a, const unsigned int p) {
		return connect(IPAddress(a,p));
	}
	bool connect(const IPAddress&);
	void update(int delay=0);
	void disconnect();

	bool connected() {
		return state == S_CONNECTED;
	}

	int send(Packet*, bool=false);
	int send(Buffer&, bool=false );
	int sendReliable(Packet*);
	int sendReliable(Buffer&);

	void setId(uint8_t i) {
		id = i;
	}
	uint8_t getId() {
		return id;
	}

	virtual void onServerConnect(const IPAddress&){};
	virtual void onServerDisconnect(){};
	virtual void onPacketReceive(Packet*){};

	ENetPeer* getENetPeer() {
		return peer;
	}

	ENetHost* getENetHost() {
		return client;
	}
};

} // namespace dnet


#endif
