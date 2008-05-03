#include <enet/enet.h>
#include <assert.h>
#include <iostream>

#include <dnet/client.hpp>
#include <dnet/buffer.hpp>
#include <dnet/packet.hpp>
#include <dnet/common.hpp>
#include <dnet/dist.hpp>

using namespace std;

namespace dnet { 

Client::Client() : state(S_DISCONNECTED) {
}

Client::~Client() {
	try {
		this->disconnect();
	}catch(...) { }
}

bool Client::connect(const IPAddress& addr) {
	if( state != S_DISCONNECTED ) return false;

	if( enet_address_set_host( &address, addr.getIP().c_str() ) != 0 ) {
		return false;
	}
	address.port = addr.getPort();

	// Create host
	client = enet_host_create( 
		NULL,
		10, // Max number of clients
		0,  // no incoming bw throttling (eg 56k modem, with 57600 / 8 for 56k downstream)
		0   // no outgping bw throttling (eg 15k modem with 14400 / 8 for 14k upstream)
	);

	if( client == NULL ) {
		return false;
	}

	int ret = enet_address_set_host( &address, addr.getIP().c_str() );
	if( ret != 0 ) {
		return false;
	}

	address.port = addr.getPort();

	peer = enet_host_connect( client, &address, 2 );
	if(! peer ) {
		enet_host_destroy(client);
		client = NULL;
		return false;
	}

	ENetEvent event;
	
	if( enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT ) {
		state = S_CONNECTING;

		// Wait 3 seconds for a Greeting (first packet send from server)
		update(3000);

		if( state == S_CONNECTED ) {
			this->onServerConnect(addr);
			return true;
		}
		else {
			// Connecting isnt fully operational - timeout. Lets give up
			rawDisconnect();
			return false;
		}
	}
	else {
		return false;
	}
}

void Client::update(int delay) {
	if( state == S_DISCONNECTED )
		return;

	dnet::DistManager::Synchronize();
	
	ENetEvent event;

	// FIXME: enet_host_service might wait for 10 seconds before returning. This MUST NOT halt Dist::Synchronize()
	while(enet_host_service(client, &event, delay ) > 0) {
		delay = 0;	// We dont want to wait next time
		switch(event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
				{
				//LOG_NET("Received %d bytes from server\n", event.packet->dataLength);
				Buffer raw(event.packet->dataLength + Packet::HEADER_SIZE);
				raw.write( event.packet->data, event.packet->dataLength );

				Packet *packet;

				if( state == S_CONNECTING ) {
					// Here, we should get a GreetingPacket
					if( (packet = PacketParser::ParsePacket(&raw)) ) {
						LOG_NET("Got packet-%d [conn]\n", packet->packetId );
						
						// GreetingPacket contains a clientId, which we need to use when generating globally unique ids
						if(GreetingPacket* gp = dynamic_cast<GreetingPacket*>(packet) ) {
							LOG_NET("Got GreetingPacket - my id: %d\n", gp->clientId);
							setId(gp->clientId);
							PacketParser::DeletePacket(packet);
							state = S_CONNECTED;
						}
						else {
							// FIXME It could store received data in a buffer and process it once the greeting arrives
							assert(0 && "Unexpected packet. We havnt gotten a GreetingPacket, but yet we get data?");
						}
					}
				}
				else {
					// We're fully connected
					while( (packet = PacketParser::ParsePacket(&raw)) ) {
						LOG_NET("Got packet-%d [full]\n", packet->packetId );
						if( DistManager::HandleClientPacket( this, packet ) )
							; // Work was done, no need to worry
						else 
							this->onPacketReceive(packet);

						PacketParser::DeletePacket(packet);
					}
				}
				enet_packet_destroy (event.packet);
				}
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				{
				state = S_DISCONNECTED;
				enet_host_destroy(client); client = NULL;
				peer = NULL;
				this->onServerDisconnect();
				return;
				}
				break;
			default:
				break;
		}
	}
}

void Client::disconnect() {
	if( state == S_DISCONNECTED ) return;
	this->onServerDisconnect();

	rawDisconnect();
}

void Client::rawDisconnect() {
	if( state == S_DISCONNECTED ) return;

	ENetEvent event;

	enet_peer_disconnect( peer );
	while(enet_host_service(client, &event, 3000) > 0) {
		switch(event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
				// Ignore packet, we dont want more stuff
				enet_packet_destroy (event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				state = S_DISCONNECTED;
				enet_host_destroy(client); client = NULL;
				peer = NULL;
				return;
			default:
				break;
		}
	}
	// Timeout - do it the brutal way
	enet_peer_reset(peer); peer = NULL;
	enet_host_destroy(client); client = NULL;
	state = S_DISCONNECTED;
}

int Client::send(Packet* packet, bool reliable) {
	Buffer buf(packet->getMaxSize() + Packet::HEADER_SIZE);

	buf << packet->packetId;	// FIXME
	*packet >> buf;				// FIXME - should be similar

	LOG_NET("Sending packet-%d to server\n", packet->packetId);
	return send(buf, reliable);
}

int Client::send(Buffer& buf, bool reliable) {
	if( state != S_CONNECTED ) return 0;
	if( buf.size() == 0 ) return 0;

	int bufSize = buf.size();

	LOG_NET("Sending %d bytes to server using %s\n", bufSize, (reliable?"reliable":"unreliable") );
	buf.dump();
	
	ENetPacket* packet = enet_packet_create(buf.getData(), bufSize, reliable ? ENET_PACKET_FLAG_RELIABLE : 0  );
	assert(packet);
	assert(peer);
	assert(client);

	return enet_peer_send(peer, 0, packet );
}

int Client::sendReliable(Packet* packet) {
	return this->send(packet,true);
}

int Client::sendReliable(Buffer& buf) {
	return this->send(buf,true);
}

} // End of dnet
