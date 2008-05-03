#include <dnet/dnet.hpp>
#include <math.h>
#include <iostream>

#include "packet.h"
#include "object.h"

using namespace std;
using namespace dnet;

class MyServer : public Server {
public:
	int		count;
	bool	game_started;
	bool	quit;
	MyServer() : Server(), count(0), game_started(false), quit(false) {
	}

    void onClientConnect(const ClientId id, const IPAddress& addr) { 
		cout << "MyServer::onClientConnect(" << id << ") from " << addr << endl; 
		count++;
    };  
	
	void onClientDisconnect(const ClientId id) { 
		cout << "MyServer::onClientDisconnect(" << id << ")" << endl; 
		count--;	
	};  

	void onPacketReceive(const ClientId id, Packet* packet) {
		if( StartGamePacket* p = dynamic_cast<StartGamePacket*>(packet) ) {
			puts("Got packet: START");
			sendReliable(p);		// Send to all clients
			game_started = true;
		}
		else if( QuitPacket* p = dynamic_cast<QuitPacket*>(packet) ) {
			puts("Got packet: QUIT");
			sendReliable(p);		// Send to all clients
			quit = true;
		}
	}
};

int main() {
	if( init() ) {
		register_packets();
		MyServer	server;
		DistManager::SetOwner(server);

		if( server.start("localhost", 2000) ) {
			while(!server.quit) {
				server.update(1000);
			}

			server.stop();
			server.update(1000);
		}
	}
}
