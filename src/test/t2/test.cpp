#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <assert.h>
#include <dnet/dnet.hpp>

using namespace std;

class MyServer : public dnet::Server {
public:
	MyServer() {
		cout << "Acting as Server" << endl;
	}
	void onClientConnect(const ClientId id, const dnet::IPAddress& addr) { 
		cout << "MyServer::onClientConnect(" << id << ") from " << addr << endl; 
	};

	void onClientDisconnect(const ClientId id) { 
		cout << "MyServer::onClientDisconnect(" << id << ")" << endl; 
	};
};

class MyClient : public dnet::Client {
public:
	MyClient() {
		cout << "Acting as Client" << endl;
	}
	void onServerConnect(const dnet::IPAddress& addr){
		cout << "MyClient::onServerConnect(" << addr << ")" << endl; 
	};
	void onServerDisconnect(){
		cout << "MyClient::onServerDisconnect()" << endl; 
	};
};

class Tank : public dnet::NetObject {
public:
	float x,y;
	uint32_t health;

	Tank() : x(0), y(0), health(0) {};
		
	DNET_DESC(Tank,
		DNET_ATTRIBUTE(x,10);
		DNET_ATTRIBUTE(y,10);
		DNET_ATTRIBUTE(health,20);
	)
};

class EvilTank : public Tank {
public:
	uint8_t iq;
	std::string	name;

	EvilTank() : Tank(), iq(3), name("GigaGun") {
		printf("health = %x\n", (unsigned int)&health );
		printf("x,y = %x,%x\n", (unsigned int)&x, (unsigned int)&y );
	}
	DNET_DESC(EvilTank,
		DNET_DESC_INHERIT(Tank);
		DNET_ATTRIBUTE(iq, 15);
	)

	void onRemotelyCreated() {
		puts("I'm ALIVE! And I'm going to take over the world! <evil haha>'");
	}

	void onRemotelyDeleted() {
		puts("Oh no! Good guy won again! But I'll be back!");
	}

	void onRemotelyUpdated() {
		printf("Evil: %d [%x]\n", health, (unsigned int)&health );
		printf("x,y: %f,%f\n", x, y );
		printf("iq : %d\n", iq );
		printf("name : %s\n", name.c_str() );
	}
};

DNET_REGISTER(Tank,50,100);
DNET_REGISTER(EvilTank,50,100);

int server() {
	MyServer server;

	dnet::DistManager::SetOwner(server);

	if( server.start("localhost", 2000) ) {
		while(1) {
			server.update(1000);
			dnet::DistManager::Synchronize();	// FIXME - should be called via from update()
		}
	}
	else {
		cerr << "Cannot start server" << endl;
		return EXIT_FAILURE;
	}

	return 0;
}

int client(const char* name) {
	MyClient client;
	
	Tank     goodTank;
	EvilTank badTank;

	dnet::DistManager::SetOwner(client);

	if( client.connect("localhost", 2000) ) {
		if( strcmp(name, "bad") == 0 )  {
			dnet::DistManager::Add(badTank);
			badTank.x = 40;
			badTank.y = 41;
			badTank.iq = 42;
		}
		else if( strcmp(name, "good") == 0 ) {
			dnet::DistManager::Add(goodTank);
		}
		else {
			puts("Unknown object");
		}
	
		for(int i = 0; i < 10; ++i ) {
			client.update(1000);
			dnet::DistManager::Synchronize();	// FIXME - should be called via from update()
			sleep(1);
			if( strcmp(name, "bad") == 0 )  {
				printf("health: %d (%x)\n", ++badTank.health, (unsigned int)&badTank.health );
			}
		}

		if( strcmp(name, "bad") == 0 )
			dnet::DistManager::Remove(badTank);
		else if( strcmp(name, "good") == 0 )
			dnet::DistManager::Remove(goodTank);
		client.update(1000);

		client.disconnect();
	}
	else {
		cerr << "Cannot connect to server" << endl;
		return EXIT_FAILURE;
	}

	return 0;
}

#define DNET_REGISTER_HACK(distclass,min,max)  { \
dnet::DistManager::ClassFunc	func; \
func.creator = (dnet::DNET_ClassDescription::CreatorFunc)&distclass::DNET_CreateInstance; \
func.deleter = (dnet::DNET_ClassDescription::DeleterFunc)&distclass::DNET_DeleteInstance; \
dnet::DistManager::NetClasses.insert(std::make_pair(std::string(#distclass), func)); \
}

int main(int /*argc*/, char* argv[]) {
	if( dnet::init() ) {
		DNET_REGISTER_HACK(Tank, 0, 0);
		DNET_REGISTER_HACK(EvilTank, 0, 0);

		try {
			if( argv[1] )
				return client(argv[1]);
			else 
				return server();
		}
		catch(const char* e) {
			puts(e);
		}
		dnet::shutdown();
	}
	else {
		cerr << "enet_initialization failed" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
