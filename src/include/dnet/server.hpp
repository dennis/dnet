#ifndef INCLUDE_DNET_SERVER_HPP
#define INCLUDE_DNET_SERVER_HPP

#include <string>
#include <map>
#include <enet/enet.h>
#include <dnet/ipaddress.hpp>
#include <dnet/interface.hpp>
#include <dnet/buffer.hpp>
#include <dnet/packet.hpp>

namespace dnet {

class Server : public Interface {
private:
	ENetAddress	address;
	ENetHost*		host;		// FIXME  Possible Resource Leak

public:
	typedef unsigned int ClientId;
	static const int MaxClientId;

protected:
	enum state_t {
		S_STOPPED, S_RUNNING
	};
	state_t	state;

public:
	struct ClientCon  {
		ClientId		id;
		ENetPeer*		peer;		// FIXME Possible Resource Leak
		unsigned int	bw;

		ClientCon() : id(0), peer(NULL), bw(0) {}
	};

	typedef std::map<ClientId,ClientCon> IdToClientMap;
	IdToClientMap idClients;

	Server();
	virtual ~Server();
	
	// Start, running, stopping
	bool start(const std::string _host, const int _port) {
		return this->start(IPAddress(_host,_port));
	}

	bool start(const IPAddress&);
	void update(int delay=0);
	void stop();
	void disconnectAll();
	void disconnectClient(ClientId);

	int send(ClientId, Packet*, bool=false);
	int send(ClientId, Buffer&, bool=false);
	int sendReliable(ClientId, Packet*);
	int sendReliable(ClientId, Buffer&);

	// Send to all
	int send(Packet*, bool=false);
	int send(Buffer&, bool=false );
	int sendReliable(Packet*);
	int sendReliable(Buffer&);

	// "Callbacks"
	virtual void onClientConnect(const ClientId, const IPAddress&){};
	virtual void onClientDisconnect(const ClientId){};
	virtual void onPacketReceive(const ClientId, Packet* ){};
	virtual void updateLOI() {};

	// Misc
	uint8_t getId() {
		return 255;
	}

	ENetHost* getENetHost() {
		return host;
	}
};

} // namespace dnet

#endif // INCLUDE_DNET_SERVER_HPP
