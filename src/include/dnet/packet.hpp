#ifndef INCLUDE_DNET_PACKET_HPP
#define INCLUDE_DNET_PACKET_HPP

#include <stdint.h>
#include <assert.h>
#include <string>

//#include <dnet/dnet.hpp>
#include <dnet/time.hpp>
#include <dnet/buffer.hpp>

namespace dnet {

class Packet {
public:
	static const uint8_t PACKET_ID;
	static const uint8_t HEADER_SIZE;
	const uint8_t packetId;

	Packet(uint8_t theId) : packetId(theId) {
	};

	virtual ~Packet()  {};

	virtual int getMaxSize() const {
		return HEADER_SIZE;
	}

	virtual Packet& operator<<(Buffer&);						// Read from buffer
	virtual const Packet& operator>>(Buffer&);			// Write to buffer
};


class PacketParser {
public:
	template<class T>
	static Packet* DefaultPacketCreateFunc() {
		return new T();
	}

	template<class T>
	static void DefaultPacketDeleteFunc(Packet* p) {
		delete p;
	}

	typedef Packet* (*PacketCreateFunc)();
	typedef void (*PacketDeleteFunc)(Packet*);

	struct PacketFuncs {
		PacketCreateFunc	createFunc;
		PacketDeleteFunc	deleteFunc;
	};

	static PacketFuncs packets[256];

	static void Initialize();

	// Packet Types
	template<class T>
	static void RegisterPacketType() {
		assert(Exists(T::PACKET_ID) == false );
		LOG_NET("RegisterPacketType() -> packet-id %d\n", T::PACKET_ID);
		packets[T::PACKET_ID].createFunc = DefaultPacketCreateFunc<T>;
		packets[T::PACKET_ID].deleteFunc = DefaultPacketDeleteFunc<T>;
		LOG_NET("  createFunc = %lx\n", (long unsigned int)packets[T::PACKET_ID].createFunc);
		LOG_NET("  deleteFunc = %lx\n", (long unsigned int)packets[T::PACKET_ID].deleteFunc);
		LOG_NET("  packets    = %lx\n", (long unsigned int)packets);
	}

	static void RemovePacketType(uint8_t);
	static bool Exists(uint8_t);

	// Packet
	static Packet* CreatePacket(uint8_t);
	static void DeletePacket(Packet* p);
	static Packet* ParsePacket(Buffer*);
	static bool InternalPacket(const Packet*);
};

class GreetingPacket : public Packet {
public:
	static const uint8_t PACKET_ID;
	uint8_t	clientId;
	
	GreetingPacket() : Packet(PACKET_ID) {};

	int getMaxSize() const {
		return sizeof(uint8_t);
	}
	
	GreetingPacket& operator<<(Buffer&);
	const GreetingPacket& operator>>(Buffer&);
};

class NetObjectCreatePacket : public Packet{
private:
	uint32_t	objId;
	std::string	className;
public:
	Buffer		buffer;
	
	static const uint8_t PACKET_ID;
	NetObjectCreatePacket() : Packet(PACKET_ID), buffer(512) {};

	void setClass(std::string name) {
		assert( name.size() < 16 );
		className = name;
	}

	const char* getClass() {
		return className.c_str();
	}

	void setObjectId(uint32_t id) {
		objId = id;
	}
	uint32_t getObjectId() {
		return objId;
	}

	int getMaxSize() const {
		return sizeof(uint32_t) + 17 + 512;		// class name is max 16 + NULL
	}
	
	NetObjectCreatePacket& operator<<(Buffer&);
	const NetObjectCreatePacket& operator>>(Buffer&);
};

class NetObjectDeletePacket : public Packet{
private:
	uint32_t	objId;

public:
	static const uint8_t PACKET_ID;
	NetObjectDeletePacket() : Packet(PACKET_ID) {};

	void setObjectId(uint32_t id) {
		objId = id;
	}
	uint32_t getObjectId() {
		return objId;
	}

	int getMaxSize() const {
		return sizeof(uint32_t);
	}
	
	NetObjectDeletePacket& operator<<(Buffer&);
	const NetObjectDeletePacket& operator>>(Buffer&);
};

class NetObjectDataPacket : public Packet {
private:
	uint32_t	objId;
	uint32_t	timestamp;

public:
	Buffer		buffer;
	
	static const uint8_t PACKET_ID;
	NetObjectDataPacket() : Packet(PACKET_ID), buffer(1024) {
		timestamp = dnettime();
	};

	void setObjectId(uint32_t id) {
		objId = id;
	}
	uint32_t getObjectId() const {
		return objId;
	}

	uint32_t getTimestamp() const {
		return timestamp;
	}

	int getMaxSize() const {
		return sizeof(uint32_t) + 512 + sizeof(uint32_t) + sizeof(uint32_t);
	}

	NetObjectDataPacket& operator<<(Buffer&);
	const NetObjectDataPacket& operator>>(Buffer&);

	NetObjectDataPacket& operator=(const NetObjectDataPacket& a) {
		this->buffer	= a.buffer;
		this->objId	= a.objId;
		this->timestamp = a.timestamp;
		return *this;
	}
};

} // namespace dnet

#endif
