#ifndef __PACKET_H__
#define __PACKET_H__

#include <dnet/dnet.hpp>

struct StartGamePacket : public dnet::Packet {
	static const uint8_t PACKET_ID;
	StartGamePacket() : Packet(PACKET_ID) {
	}
};

struct QuitPacket : public dnet::Packet {
	static const uint8_t PACKET_ID;
	QuitPacket() : Packet(PACKET_ID) {
	}
};

const uint8_t StartGamePacket::PACKET_ID = 0;
const uint8_t QuitPacket::PACKET_ID = 1;

void register_packets() {
	dnet::PacketParser::RegisterPacketType<StartGamePacket>();
	dnet::PacketParser::RegisterPacketType<QuitPacket>();
	
}
#endif
