#include <dnet/packet.hpp>
#include <dnet/buffer.hpp>
#include <dnet/common.hpp>
#include <assert.h>

namespace dnet {

const uint8_t Packet::HEADER_SIZE = 1;

const uint8_t Packet::PACKET_ID = 0;

const uint8_t GreetingPacket::PACKET_ID			= 200;
const uint8_t NetObjectCreatePacket::PACKET_ID	= 201;
const uint8_t NetObjectDeletePacket::PACKET_ID	= 202;
const uint8_t NetObjectDataPacket::PACKET_ID	= 203;

PacketParser::PacketFuncs PacketParser::packets[256];

Packet& Packet::operator<<(Buffer&) {
	return *this;
}

const Packet& Packet::operator>>(Buffer&)  {
	return *this;
}

void PacketParser::Initialize() {
	for(int i = 0; i < 256; i++ ) {
		packets[i].createFunc = 0L;
		packets[i].deleteFunc = 0L;
	}
	// Register Internal PacketTypes here
	RegisterPacketType<GreetingPacket>();
	RegisterPacketType<NetObjectCreatePacket>();
	RegisterPacketType<NetObjectDeletePacket>();
	RegisterPacketType<NetObjectDataPacket>();
}

void PacketParser::RemovePacketType(uint8_t id) {
	assert(!Exists(id));
	packets[id].createFunc = 0L;
	packets[id].deleteFunc = 0L;
}

bool PacketParser::Exists(uint8_t id) {
	return packets[id].createFunc != 0L;
}

Packet* PacketParser::CreatePacket(uint8_t id) {
	LOG_NET("CreatePacket(%d)\n", id);
	LOG_NET("  createFunc = %lx\n", (long unsigned int)packets[id].createFunc);
	LOG_NET("  deleteFunc = %lx\n", (long unsigned int)packets[id].deleteFunc);
	LOG_NET("  packets    = %lx\n", (long unsigned int)packets);
	PacketCreateFunc	func = packets[id].createFunc;
	assert(func);

	return func();
}

void PacketParser::DeletePacket(Packet* p) {
	if( p ) {
		PacketDeleteFunc	func = packets[p->packetId].deleteFunc;
		assert(func);
		func(p);
	}
}

Packet* PacketParser::ParsePacket(Buffer* raw) {
	if( raw->size() == 0 ) return 0L;
	uint8_t id = 0;		// Packet ID

	LOG_NET("ParsePacket: %d bytes\n", raw->size());
	raw->dump();
	*raw >> id;

	Packet* newPacket = CreatePacket(id);

	if(! newPacket ) {
		LOG_NET("Recieved invalid packet type %d\n", id);
		raw->clear();
		return NULL;
	}

	try {
		LOG_NET("Parsing payload..\n");
		(*newPacket) << *raw;
	}
	catch( ... ) {
		DeletePacket(newPacket);
		LOG_NET("Exception thrown!\n");
		return NULL;
	}

	return newPacket;
}

bool PacketParser::InternalPacket(const Packet* p) {
	return( p->packetId >= 200 );
}

GreetingPacket& GreetingPacket::operator<<(Buffer& buf) {		// Read from buffer
	buf >> clientId;
	return *this;
}

const GreetingPacket& GreetingPacket::operator>>(Buffer& buf)  {	// Write to buffer
	buf << clientId;
	return *this;
}

NetObjectCreatePacket& NetObjectCreatePacket::operator<<(Buffer& buf) {		// Read from buffer
	buf >> objId >> className;
	buf >> buffer;
	return *this;
}

const NetObjectCreatePacket& NetObjectCreatePacket::operator>>(Buffer& buf) {	// Write to buffer
	buf << objId << className;
	buf << buffer;
	return *this;
}

NetObjectDeletePacket& NetObjectDeletePacket::operator<<(Buffer& buf) {		// Read from buffer
	buf >> objId;
	return *this;
}

const NetObjectDeletePacket& NetObjectDeletePacket::operator>>(Buffer& buf) {	// Write to buffer
	buf << objId;
	return *this;
}

NetObjectDataPacket& NetObjectDataPacket::operator<<(Buffer& buf) {		// Read from buffer
	buf >> objId;
	buf >> timestamp;
	buf >> buffer;
	return *this;
}

const NetObjectDataPacket& NetObjectDataPacket::operator>>(Buffer& buf) {	// Write to buffer
	assert( buffer.size() < 512 );
	buf << objId;
	buf << timestamp;
	buf << buffer;
	return *this;
}

} // namespace dnet
