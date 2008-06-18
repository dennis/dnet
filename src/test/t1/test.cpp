#include <dnet/dnet.hpp>
#include <iostream>
#include <assert.h>
#include <string.h>

using namespace std;

class DataPacket : public dnet::Packet {
	char*	data;
	int		len;
public:
	static const uint8_t ID;
	DataPacket() : Packet(ID), data(static_cast<char*>(malloc(1024))), len(1024) {
	}

	DataPacket(char* src, int srcl) : Packet(ID) {
		cout << "DataPacket::DataPacket() len = " << srcl << endl;
		data = static_cast<char*>(malloc(srcl));
		memcpy(data, src, srcl);
		len = srcl;
	}

	const char* getData() {
		return data;
	}

	~DataPacket() {
		free(data);
	}

	int getMaxSize() const {
		return 1024;
	}

	DataPacket& operator<<(dnet::Buffer& buf) {		// Read from buffer
		buf.read((uint8_t*)data,buf.size());
		return *this;
	}

	const DataPacket& operator>>(dnet::Buffer& buf) const {	// Write to buffer
		buf.write((uint8_t*)data,len);
		return *this;
	}
};

const uint8_t DataPacket::ID = 0;

class MyServer : public dnet::Server {
public:
	MyServer() {
		cout << "Acting as Server" << endl;
	}
	void onClientConnect(const ClientId id, const dnet::IPAddress& addr) { 
		cout << "MyServer::onClientConnect(" << id << ") from " << addr << endl; 
		DataPacket data("Hello there",12);
		sendReliable(id,&data);
	};

	void onClientDisconnect(const ClientId id) { 
		cout << "MyServer::onClientDisconnect(" << id << ")" << endl; 
	};

	void onPacketReceive(const ClientId id, dnet::Packet* packet) {
		printf("MyServer: Got Packet(%d)\n", packet->PACKET_ID);
		DataPacket*	dp = static_cast<DataPacket*>(packet);
		puts(dp->getData());
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

	void onPacketReceive(dnet::Packet* packet){ // FIXME
		printf("MyClient: Got Packet(%d)\n", packet->PACKET_ID);
		DataPacket*	dp = static_cast<DataPacket*>(packet);
		puts(dp->getData());
	};
};

int main(int argc, char* argv[]) {
	if( argc < 2 )  {
		cerr << "usage: " << argv[0] << " [send,receive]" << endl;
		return EXIT_FAILURE;
	}
	if( dnet::init() ) {
		dnet::PacketParser::RegisterPacketType<DataPacket>();
		try {
			if( argv[1][0] == 'r' ) {
				MyServer server;

				if( server.start("localhost", 2000) ) {
					server.update(1000);

					while(1) {
						server.update(1000);
					}
				}
				else {
					cerr << "Cannot start server" << endl;
					return EXIT_FAILURE;
				}
			}
			else {
				MyClient client;

				if( client.connect("localhost", 2000) ) {
					client.update(3000);
					{
						DataPacket data("Hello world",12);
						client.sendReliable(&data);
					}
	#define BUFLEN 32
					/*
					char buf[BUFLEN]; 
					bool eof = false;
					int c;
					int i;
					do {
						client.update();
						for(i = 0; i < BUFLEN; i++ ) {
							c = getchar();
							if( c == EOF) {
								eof = true;
								break;
							}
							else {
								buf[i] = c;
							}
						}
						{
							DataPacket data(buf,i);
							client.sendReliable(&data);
						}
					}while(!eof && client.connected() );
					*/

					if( client.connected() ) {
						client.update(1000);
						client.disconnect();
					}
					else {
						puts("We lost the server...");
					}
				}
				else {
					cerr << "Cannot connect to server" << endl;
					return EXIT_FAILURE;
				}
			}
			dnet::shutdown();
		}
		catch( char const* error ) {
			puts(error);
		}
		catch(...) {
			puts("Unknown exception");
		}
	}
	else {
		cerr << "enet_initialization failed" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
