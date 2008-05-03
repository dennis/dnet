#include <dnet/dnet.hpp>
#include "common.h"

#include <math.h>
#include <iostream>

using namespace std;

class MyServer : public dnet::Server {
    void onClientConnect(const ClientId id, const dnet::IPAddress& addr) { 
		cout << "MyServer::onClientConnect(" << id << ") from " << addr << endl; 
    };  
	
	void onClientDisconnect(const ClientId id) { 
		cout << "MyServer::onClientDisconnect(" << id << ")" << endl; 
	};  

	void updateLOI() {
		float dx, dy, dist;
		for(SpaceshipsVector_t::iterator i = spaceships.begin(); i != spaceships.end(); ++i ) {
			for(SpaceshipsVector_t::iterator j = spaceships.begin(); j != spaceships.end(); ++j ) {
				if( i == j ) continue;
				
				dx = (*i)->x - (*j)->x;
				dy = (*i)->y - (*j)->y;
				dist = sqrtf(dx*dx+dy*dy);

				if( dist > 100 )
					dnet::DistManager::SetLOI((*i)->getClientId(), (*j)->getObjectId(), 0.5);
				else
					dnet::DistManager::SetLOI((*i)->getClientId(), (*j)->getObjectId(), 1);
			}
		}
	}
};

int main() {
	if( dnet::init() ) {
		MyServer	server;
		dnet::DistManager::SetOwner(server);

		if( server.start("localhost", 2000) ) {
			while(1) {
				server.update(1000);
			}
		}
	}
}
