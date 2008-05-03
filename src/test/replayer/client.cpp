#include <dnet/dnet.hpp>
#include "packet.h"
#include "history.h"
#include "object.h"

using namespace dnet;

class MyClient : public Client {
public:
	enum client_state_t { IDLE, START, QUIT };
	client_state_t	state;
	int				rtt;

	MyClient() : state(IDLE) {
	}

	void onPacketReceive(Packet* packet) {
		if( dynamic_cast<StartGamePacket*>(packet) ) {
			puts("Got packet: START");
			state = START;
			rtt = getENetPeer()->roundTripTime;
		}
		else if( dynamic_cast<QuitPacket*>(packet) ) {
			puts("Got packet: QUIT");
			state = QUIT;
		}
	}
};

int main(int argc, char* argv[]) {
	if( dnet::init() ) {
		try {
			register_packets();
			MyClient	client;
			DistManager::SetOwner(client);

			if(! client.connect("localhost",2000) ) {
				puts("Failed connecting to server");
				return 1;
			}
			
			int my_id;
			printf("argc : %d\n", argc);
			if( argc == 2 )
				my_id = atoi(argv[1]);
			else
				my_id = 0;

			printf("Reading bzflag.log for id %d\n", my_id );
			History* hist = new History("bzflag.log", my_id);
			printf("..done. %d iterations\n", hist->iters());

			while(client.state ==  client.IDLE) 
				client.update(1000);
			
			// Play file
			ReplayTank	tank;
			unsigned int start = dnettime() - (client.rtt/2);
			unsigned int elapsed = 0;
			int i = 0;
			DistManager::Add(tank);
			while(client.state == client.START) {
				elapsed = dnettime() - start;

				while( hist->iter(i).t()*1000 < elapsed ) {
					// change position of object
					if( hist->iter(i).numTanks() > 0 ) {
						tank = hist->iter(i).getTank(0);
						printf(".");
					}
					++i;

					if( i > hist->iters() ) 
						client.state = client.QUIT;
				}

				printf("Elapsed: %d\n", elapsed );
				client.update(100);
			}

			DistManager::Remove(tank);
			puts("Disconnecting");

			client.disconnect();
			delete hist;

		}
		catch(const std::string& t) {
			puts(t.c_str());
		}
		catch(...) {
			puts("catched ...");
		}
	}
	else {
		puts("DNET initialization failed");
	}
}
