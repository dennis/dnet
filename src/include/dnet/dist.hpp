#ifndef INCLUDE_DNET_DIST_HPP
#define INCLUDE_DNET_DIST_HPP

#include <dnet/common.hpp>
#include <dnet/packet.hpp>
#include <dnet/buffer.hpp>
#include <dnet/pack.hpp>

#include <vector>
#include <stdint.h>
#include <assert.h>
#include <string>
#include <string.h>
#include <map>
#include <enet/enet.h>

namespace dnet {

class Interface;
class Server;
class Client;
class NetObject;

class Attribute {
private:
	enum AttributeType {
		INT8, UINT8, INT16, UINT16, INT32, UINT32, FLOAT, STRING
	};

	uint8_t 				id;
	const char* 			name;
	int 						size;
	AttributeType		type;
	int						priority;
	int						offset;
	uint8_t*				data;
public:
	Attribute(int i, const char* n, uint32_t off, int pri, size_t s,  int8_t* d) : id(i), name(n), size(s), type(INT8), priority(pri), offset(off), data(reinterpret_cast<uint8_t*>(d)) { assert(id < 32); 	}
	Attribute(int i, const char* n, uint32_t off, int pri, size_t s,  uint8_t* d) : id(i), name(n), size(s), type(UINT8), priority(pri), offset(off), data(reinterpret_cast<uint8_t*>(d))   { assert(id < 32); }
	Attribute(int i, const char* n, uint32_t off, int pri, size_t s, int16_t* d) : id(i), name(n), size(s), type(INT16), priority(pri), offset(off), data(reinterpret_cast<uint8_t*>(d))   { assert(id < 32);	}
	Attribute(int i, const char* n, uint32_t off, int pri, size_t s, uint16_t* d) : id(i), name(n), size(s), type(UINT16), priority(pri), offset(off), data(reinterpret_cast<uint8_t*>(d))   { assert(id < 32); }
	Attribute(int i, const char* n, uint32_t off, int pri, size_t s, int32_t* d) : id(i), name(n), size(s), type(INT32), priority(pri), offset(off), data(reinterpret_cast<uint8_t*>(d))   { assert(id < 32); }
	Attribute(int i, const char* n, uint32_t off, int pri, size_t s, uint32_t* d) : id(i), name(n), size(s), type(UINT32), priority(pri), offset(off), data(reinterpret_cast<uint8_t*>(d))   { assert(id < 32); }
	Attribute(int i, const char* n, uint32_t off, int pri, size_t s, float* d) : id(i), name(n), size(s), type(FLOAT), priority(pri), offset(off), data(reinterpret_cast<uint8_t*>(d))   { assert(id < 32); }
	
	Attribute(int i, const char* n, uint32_t off, int pri, size_t s, std::string* d) : id(i), name(n), size(s), type(STRING), priority(pri), offset(off), data(reinterpret_cast<uint8_t*>(d))  { assert(id < 32); }

	void dump() {
		printf("Attribute: name: %s, offset: %d, priority: %d, size: %d, type: %d, data: %x\n", 
			this->name, 	this->offset, 
			this->priority, this->size, 
			this->type, 	(unsigned int)this->data);
	}

	const char* getName() const {
		return this->name;
	}

	void pack(Buffer* buffer) {
		uint8_t  raw[512];	// Packto buffer
		uint8_t* ptr = raw;
		LOG_PACK("packing %s to buffer (buffer size = %d). source = %x, size = %d\n",name, buffer->size(), (unsigned int)data, size );
		switch(type) {
			case INT8: 		ptr = (uint8_t*)::dnet::pack(ptr,*reinterpret_cast<int8_t*>(data));		break;
			case UINT8:		ptr = (uint8_t*)::dnet::pack(ptr,*reinterpret_cast<uint8_t*>(data));	break;
			case INT16:		ptr = (uint8_t*)::dnet::pack(ptr,*reinterpret_cast<int16_t*>(data));	break;
			case UINT16:	ptr = (uint8_t*)::dnet::pack(ptr,*reinterpret_cast<uint16_t*>(data));	break;
			case INT32:		ptr = (uint8_t*)::dnet::pack(ptr,*reinterpret_cast<int32_t*>(data));	break;
			case UINT32:	ptr = (uint8_t*)::dnet::pack(ptr,*reinterpret_cast<uint32_t*>(data));	break;
			case FLOAT:		ptr = (uint8_t*)::dnet::pack(ptr,*reinterpret_cast<float*>(data));		break;
			case STRING:	ptr = (uint8_t*)::dnet::pack(ptr,*reinterpret_cast<std::string*>(data));		break;
		}
		assert( (ptr-raw) == size );
		buffer->write(raw,size);
		LOG_PACK("Added %d bytes to buffer. Total size %d\n", size, buffer->size());
	}
	
	void unpack(Buffer* buffer) {
		uint8_t* raw = buffer->peekData();
		uint8_t* ptr = raw;
		
		LOG_PACK("unpacking %s bytes from buffer. source = %x, target = %x, size = %d, buffersize = %d\n", name, (unsigned int)raw, (unsigned int)data, size, buffer->size() );
		assert(buffer->size()>0);
		//buffer->dump();
		
		switch(type) {
			case INT8:			ptr = (uint8_t*)::dnet::unpack(ptr,*reinterpret_cast<int8_t*>(data));			break;
			case UINT8:		ptr = (uint8_t*)::dnet::unpack(ptr,*reinterpret_cast<uint8_t*>(data));		break;
			case INT16:		ptr = (uint8_t*)::dnet::unpack(ptr,*reinterpret_cast<int16_t*>(data));		break;
			case UINT16:		ptr = (uint8_t*)::dnet::unpack(ptr,*reinterpret_cast<uint16_t*>(data));		break;
			case INT32:		ptr = (uint8_t*)::dnet::unpack(ptr,*reinterpret_cast<int32_t*>(data));		break;
			case UINT32:		ptr = (uint8_t*)::dnet::unpack(ptr,*reinterpret_cast<uint32_t*>(data));		break;
			case FLOAT:		ptr = (uint8_t*)::dnet::unpack(ptr,*reinterpret_cast<float*>(data));			break;
			case STRING:		ptr = (uint8_t*)::dnet::unpack(ptr,*reinterpret_cast<std::string*>(data));	break;
		}
		
		buffer->discard( size );

		LOG_PACK(" After discard buffer is %d bytes\n", buffer->size());
	}

	inline int getPriority() const {
		return priority;
	}

	inline uint8_t getID() const {
		return id;
	}
};

struct AttributeSorter {
	bool operator()(const Attribute& a, const Attribute& b) {
		if( a.getPriority() == b.getPriority() )
			return a.getID() > b.getID();
		else
			return a.getPriority() > b.getPriority();
	}
};

class DNET_ClassDescription {
public:
	typedef NetObject* (*CreatorFunc)();
	typedef void (*DeleterFunc)(NetObject*);
private:
	uint32_t		min;
	uint32_t		max;
	std::string	name;
	CreatorFunc	creator;
	DeleterFunc deleter;

	friend class DistManager;

public:
	DNET_ClassDescription() {};
	DNET_ClassDescription(char const* n, int min_interval, int max_interval, CreatorFunc theCreator, DeleterFunc theDeleter );

	const char* getName() const {
		return name.c_str();
	}

	uint32_t getMinInterval() {
		return min;
	}

	uint32_t getMaxInterval() {
		return max;
	}
};

typedef std::vector< dnet::Attribute > Attributes;

class DistManager {
public:
	struct ClassFunc {
		DNET_ClassDescription::CreatorFunc	creator;
		DNET_ClassDescription::DeleterFunc 	deleter;
	};
private:
	static Interface*	interface;
	static bool			isServer;
	static uint32_t		id;

	static uint32_t GenId();

	static void SendCreationPacket(uint32_t, const DNET_ClassDescription&);
	static void SendDeletionPacket(uint32_t, const DNET_ClassDescription&);

	
	typedef std::map<std::string, ClassFunc> ClassMap;
	typedef std::map<uint32_t, NetObject* > ObjectMap;
	typedef std::map<uint8_t, std::map<uint32_t,float> > LOIMap;
public:
	static ClassMap		NetClasses;
	static ObjectMap		NetObjects;
	static LOIMap			NetObjectsLOI;

public:
	static void SetOwner(Server&);
	static void SetOwner(Client&);
		
	static bool CheckClientId(uint32_t id, uint8_t owner);
	static bool DoSync(uint32_t id, uint8_t owner);

	static void SetLOI(uint8_t, uint32_t, float );
	static float GetLOI(uint8_t, uint32_t);

	static void Add(NetObject&);
	static void Remove(NetObject&);

	static void RegisterClass(const char* name, DNET_ClassDescription::CreatorFunc, DNET_ClassDescription::DeleterFunc);
	
	static NetObject* CreateObject(const char*, uint32_t,bool=true);
	static NetObject* CreateObject(NetObject*);
	static void DeleteObject(uint32_t);
	static void UpdateObject(uint32_t,uint32_t,Buffer*);

	static void Synchronize();
	static void SynchronizeSimple();
	static void SynchronizeASBS();
	static void SynchronizeASBSFor(uint32_t,ENetHost*, ENetPeer*);

	static void SendAllNetObjects(Server*,uint8_t);
	static bool HandleServerPacket(Server*,Packet*, uint8_t);
	static bool HandleClientPacket(Client*,Packet*);

	static void ClientDisconnected(uint8_t);
};

class NetObject  {
protected:
	bool			dynamicAllocated;			// Use delete to free object
	uint32_t	objectId;							// Unique global identifier
	uint32_t	lastTimestamp;				// Of last packet received (To detect OoO packets)
	bool			local;								// true = the object was created by "us", false = object was created via DNet
	uint32_t	lastFullSynch;					// Ticks
	uint32_t	lastSynch;						// Ticks
	
public:
	friend class DistManager;
	
	NetObject() : dynamicAllocated(false), objectId(0), local(true), lastFullSynch(0), lastSynch(0) {}
	virtual ~NetObject() {};
	
	virtual DNET_ClassDescription* DNET_GetClassDescription() = 0;		// Implemented via macros
	
	virtual Attributes* DNET_GetAttributes() = 0;			// Implemented via macros
	virtual void DNET_Attributes(Attributes*) = 0;			// Implemented via macros

	NetObject& operator<<(Buffer&) { return *this; }
	const NetObject& operator>>(Buffer&) const { return *this; }

	bool isLocal() {
		return local;
	}

	uint32_t getObjectId() {
		return objectId;
	}

	uint8_t getClientId() {
		return (objectId>>24);
	}

	virtual void onRemotelyCreated() {};
	virtual void onRemotelyDeleted() {};
	virtual void onRemotelyUpdated() {};

#ifdef DEBUG
	virtual void debug() {};
#endif
};

// FIXME DNET_GetAttributes() is VERY inefficient - we create the list for every time this method is called :(
#define DNET_DESC(distclass,...) \
	static dnet::DNET_ClassDescription DNET_ClassDescriptor;  \
	void DNET_Attributes(dnet::Attributes* list) { \
		int i =1; \
		__VA_ARGS__ \
	} \
	static distclass* DNET_CreateInstance() { \
		return new distclass(); \
	} \
	static void DNET_DeleteInstance(NetObject* p) { \
		delete p; \
	} \
	dnet::Attributes* DNET_GetAttributes() { \
		dnet::Attributes* list = NULL; \
		if(! list ) { \
			list = new dnet::Attributes(); \
			DNET_Attributes(list); \
			std::sort(list->begin(), list->end(), dnet::AttributeSorter() ); \
		} \
		return list; \
	} \
	dnet::DNET_ClassDescription* DNET_GetClassDescription() { \
		return &DNET_ClassDescriptor; \
	}
	// FIXME DNET_GetAttributes() is a leak!
#define DNET_ATTRIBUTE(name,pri) list->push_back(dnet::Attribute(i++,#name,(uint32_t)&name-(uint32_t)this,pri,dnet::SizeOf(name), &name ))
#define DNET_DESC_INHERIT(parent)  parent::DNET_Attributes(list)
#define DNET_REGISTER(distclass,min,max) dnet::DNET_ClassDescription distclass::DNET_ClassDescriptor(#distclass,min,max, (dnet::DNET_ClassDescription::CreatorFunc)&distclass::DNET_CreateInstance,(dnet::DNET_ClassDescription::DeleterFunc)&distclass::DNET_DeleteInstance)
} // namespace dnet

#endif
