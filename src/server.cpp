#include <string>
#include <iostream>
#include <assert.h>

#include <dnet/common.hpp>
#include <dnet/server.hpp>
#include <dnet/dnet.hpp>

using namespace std;

namespace dnet {

const int Server::MaxClientId = 255; // Everything lower than 255 is allright

Server::Server() : state(S_STOPPED) { 
}

Server::~Server() { 
	if( host ) enet_host_destroy(host);
	disconnectAll();
} 

bool Server::start(const IPAddress& addr) {

	assert( state == S_STOPPED );

	if( enet_address_set_host( &address, addr.getIP().c_str() ) != 0 ) {
		cerr << "enet_address_set_host() failed" << endl;
		return false;
	}
	address.port = addr.getPort();

	// Create host
	host = enet_host_create( &address, 
		255, // Max number of clients
		0,  // no incoming bw throttling (eg 56k modem, with 57600 / 8 for 56k downstream)
		0   // no outgping bw throttling (eg 15k modem with 14400 / 8 for 14k upstream)
	);

	if( host == NULL ) {
		cerr << "enet_host_create() failed" << endl;
		return false;
	}

	state = S_RUNNING;

	return true;
}
void Server::update(int delay) {

	if( state != S_RUNNING) 
		return;

	dnet::DistManager::Synchronize();

	// Main loop of ENet
	ENetEvent event;

	// FIXME: enet_host_service might wait for 10 seconds before returning. This MUST NOT halt Dist::Synchronize()
	while( enet_host_service(host, &event, delay ) > 0 ) {

		LOG_NET("Server got %d clients\n", idClients.size());

		delay = 0;
		
		switch(event.type) {

		case ENET_EVENT_TYPE_CONNECT:
			assert( event.peer );
			if( state == S_RUNNING ) {
				// Find a free client ID
				ClientId nextId = 0;

				int i;
				for(i = 0; i < MaxClientId; i++){
					if( idClients.count(i) == 0 ) {
						nextId = i;
						break;
					}
				}

				// No available client ID: Disconnect it brutally
				if( i == MaxClientId ) {
					enet_peer_reset(event.peer); 
					return;
				}

				ClientCon sender;
				sender.peer		= event.peer;
				sender.id  		= nextId;
				event.peer->data = reinterpret_cast<void*>(sender.id);

				idClients.insert( std::make_pair( sender.id, sender ) );

				// Send GreetingPacket
				GreetingPacket	hello;
				hello.clientId = sender.id;
				sendReliable(sender.id,&hello);

				// Notify that we got a new client
				this->onClientConnect(sender.id, IPAddress( event.peer->address.host, event.peer->address.port));

				// Bring client up to speed
				DistManager::SendAllNetObjects(this,sender.id);
			}
			break;

		case ENET_EVENT_TYPE_RECEIVE:
			assert( event.peer );
			assert( event.packet );
			assert( event.packet->dataLength > 0 );
			assert( event.packet->data );
	
			if( state == S_RUNNING ) {
				ClientCon& sender = idClients[ reinterpret_cast<ClientId>(event.peer->data) ];

				LOG_NET("Received %d bytes from client %d\n", event.packet->dataLength, sender.id );

				assert( event.packet->dataLength >= 0 );
		
				Buffer raw(event.packet->dataLength + Packet::HEADER_SIZE);
				raw.write( event.packet->data, event.packet->dataLength );

				Packet *packet;

				while( (packet = PacketParser::ParsePacket(&raw)) ) {
					LOG_NET("Handling packet-%d\n", packet->packetId);
					if( DistManager::HandleServerPacket(this,packet,sender.id) )
						; // Yay, DistManager did whatever it needed to
					else
						this->onPacketReceive(sender.id, packet);
					PacketParser::DeletePacket(packet);
				}

				if( raw.size() ) {
					LOG_NET("Parsing not finished - still %d bytes\n", raw.size());
				}
			}
			else {
				LOG_NET("Server is not in running state - packet ignored\n");
			}
			enet_packet_destroy(event.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			assert( event.peer );
			disconnectClient(reinterpret_cast<ClientId>(event.peer->data));
			break;

		case ENET_EVENT_TYPE_NONE:
			break;
		}
	}
}

void Server::stop() {
	if( state != S_RUNNING ) return;

	enet_host_destroy(host);
	host = NULL;
		
	state = S_STOPPED;
}

void Server::disconnectClient(ClientId clientId) {
	if( state != S_RUNNING ) return;
	
	ClientCon client = idClients[clientId];
	
	assert(client.peer);
	this->onClientDisconnect(clientId);
 	enet_peer_disconnect(client.peer);
	idClients.erase(clientId);	// This needs to be before ClientDisconnect, otherwise it got clientId (the deleted one) as an available client

	DistManager::ClientDisconnected(clientId);
}

void Server::disconnectAll() {
	// Disconnect all clients
	IdToClientMap::iterator	i;
	 
	 for( i = idClients.begin(); i != idClients.end(); ++i )
	 	disconnectClient(i->first);
}

int Server::send(ClientId clientId, Packet* packet, bool reliable) {
	if( state != S_RUNNING ) return 0;

	Buffer buf(packet->getMaxSize() + Packet::HEADER_SIZE);

	buf << packet->packetId;		// FIXME
	*packet >> buf;						// FIXME - should be similar

	LOG_NET("Sending packet-%d to client %d, %d bytes\n", packet->packetId, clientId, buf.size());
	buf.dump();
	return send(clientId, buf, reliable );
}

int Server::send(ClientId clientId, Buffer &buf, bool reliable) {
	if( state != S_RUNNING ) return 0;
	if( buf.size() == 0 ) return 0;

	int bufSize = buf.size();

	//LOG_NET("Sending %d bytes to %d using %s\n", bufSize, clientId, (reliable?"reliable":"unreliable") );
	ENetPacket* packet = enet_packet_create(buf.getData(), bufSize, reliable ? ENET_PACKET_FLAG_RELIABLE : 0  );
	assert(packet);
	assert(idClients[clientId].peer);
	assert(host);

	return enet_peer_send(idClients[clientId].peer, 0, packet );
	//enet_host_flush(host);
}

int Server::sendReliable(ClientId clientId, Packet* packet) {
	return this->send(clientId,packet,true);
}

int Server::sendReliable(ClientId clientId, Buffer& buf) {
	return this->send(clientId,buf,true);
}

int Server::send(Packet* packet, bool reliable) {
	if( state != S_RUNNING ) return false;

	Buffer buf(packet->getMaxSize() + Packet::HEADER_SIZE);

	buf << packet->packetId;		// FIXME
	*packet >> buf;						// FIXME - should be similar

	return send(buf, reliable );
}

int Server::send(Buffer& buf, bool reliable ) {
	// Send it to everyone
	IdToClientMap::iterator	i;
	
	int total = 0;
	for( i = idClients.begin(); i != idClients.end(); ++i )
	 	total += send(i->first,buf, reliable );
	// FIXME In case a send fails, what should we do? Currently we simple ignore the case
	
	return total;
}


int Server::sendReliable(Packet* packet) {
	return this->send(packet,true);
}

int Server::sendReliable(Buffer& buf) {
	return this->send(buf,true);
}

} // namespace dnet
