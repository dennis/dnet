#include <dnet/dnet.hpp>
#include "packet.h"

using namespace dnet;

void packet_init() {
}
int main(int argc, char* argv[]) {
	if( argc == 0 ) {
		fprintf(stderr, "%s <command>\n", argv[0] );
		return 1;
	}
	if( dnet::init() ) {
		register_packets();
		Client	client;
		DistManager::SetOwner(client);

		if(! client.connect("localhost",2000) ) {
			puts("Failed connecting to server");
			return 1;
		}

		client.update(1000);

		if( !strcmp(argv[1], "start") ) { 
			puts("Sending StartGamePacket");
			StartGamePacket	s;
			client.sendReliable(&s);
		}
		else if( !strcmp(argv[1], "quit") ) { 
			puts("Sending QuitPacket");
			QuitPacket	p;
			client.sendReliable(&p);
		}

		client.update(1000);
		client.disconnect();
	}
	else {
		puts("DNET initialization failed");
	}
}
