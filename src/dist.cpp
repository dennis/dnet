#include <dnet/dist.hpp>
#include <dnet/server.hpp>
#include <dnet/client.hpp>
#include <stdio.h>
#include <assert.h>
#include <dnet/common.hpp>
#include <math.h>

#include <list>
#include <functional>

namespace dnet {

uint32_t 									DistManager::id								= 0;
DistManager::ClassMap				DistManager::NetClasses;
DistManager::ObjectMap				DistManager::NetObjects;
DistManager::LOIMap					DistManager::NetObjectsLOI;
Interface* 									DistManager::interface					= 0L;
bool 											DistManager::isServer					= false;

DNET_ClassDescription::DNET_ClassDescription(char const* n, int min_interval, int max_interval, CreatorFunc theCreator, DeleterFunc theDeleter ) : min(min_interval), max(max_interval), name(n), creator(theCreator), deleter(theDeleter) {
	LOG_DIST("DNET_ClassDescription(%s)\n", n);
	assert(min <= max);
	DistManager::RegisterClass(n,creator,deleter);
}

void DistManager::SetOwner(Server& s) {
	interface = &s;
	isServer = true;
}

void DistManager::SetOwner(Client& c) {
	interface = &c;
	isServer = false;
}

uint32_t DistManager::GenId() {
	// TODO Use the Interface class
	uint32_t p = interface->getId();

	p = p << 24;

	p |= (++id & 0x00ffff);
	return p;
}

bool DistManager::CheckClientId(uint32_t id, uint8_t owner) {
	return (  (id>>24) == owner );
}

void DistManager::SendCreationPacket(uint32_t objectId, const DNET_ClassDescription& desc) {
	LOG_DIST("Sending NetObjectCreatePacket -> CreateObject(%s#%x)\n", desc.getName(), objectId );
	NetObjectCreatePacket	create;
	create.setClass(desc.getName());
	create.setObjectId(objectId);
	interface->sendReliable(&create);
}

void DistManager::SendDeletionPacket(uint32_t objectId, const DNET_ClassDescription& desc) {
	LOG_DIST("DeleteObject(%s#%x)\n", desc.getName(), objectId );
	NetObjectDeletePacket	die;
	die.setObjectId(objectId);
	interface->sendReliable(&die);
}

NetObject* DistManager::CreateObject(const char* name, uint32_t objectId, bool local) {
	ClassMap::iterator i = NetClasses.find(name);

	if( i != NetClasses.end() ) {
		DNET_ClassDescription* desc;
		DNET_ClassDescription::CreatorFunc creator = i->second.creator;

		NetObject* nObj = creator();
		
		LOG_DIST("CreateObject(%s#%x) created : %x\n", name, objectId, (unsigned int)nObj);
		
		desc = nObj->DNET_GetClassDescription();
		nObj->dynamicAllocated = true;
		nObj->objectId = objectId;
		nObj->local = local;
		
		NetObjects.insert( std::make_pair( objectId, nObj ));
		
		//if(! CheckClientId( objectId, interface->getId() ) )
		// FIXME This CreateObject() function is only called for remote objects
		if(! local )
			nObj->onRemotelyCreated();

		return nObj;
	}
	else {
		LOG_DIST("CreateObject(%s#%x) UNKNOWN\n", name, objectId);
	}
	
	return 0L;
}

// Add a new object. We dont perform allocation/removal
NetObject*  DistManager::CreateObject(NetObject* nObj) {
	LOG_DIST("CreateNetObject(%s#%x) [static]  created : %x\n", nObj->DNET_GetClassDescription()->getName(), nObj->objectId, (unsigned int)nObj);
	NetObjects.insert( std::make_pair( nObj->objectId, nObj ));
	return nObj;
}

void DistManager::DeleteObject(uint32_t id) {
	ObjectMap::iterator i = NetObjects.find(id);

	if( i != NetObjects.end() ) {
		NetObject* nObj = i->second;
		LOG_DIST("DeleteNetObject(%x) deleting: %x\n", id, (unsigned int)nObj );

		if(! nObj->isLocal() )
			nObj->onRemotelyDeleted();
		
		NetObjects.erase(i);

		if( nObj->dynamicAllocated ) {
			DNET_ClassDescription::DeleterFunc deleter = nObj->DNET_GetClassDescription()->deleter;
			deleter(i->second); // Not sure this is working properly
		}
	}
	else {
		LOG_DIST("DeleteObject - object %x not found\n", id);
	}
}

struct AttributeFinderByID {
	typedef Attribute	first_argument_type;
	typedef uint8_t		second_argument_type;
	typedef bool			result_type;

	bool operator()(const Attribute& attr, uint8_t id) const {
		return attr.getID() == id;
	}
};

void DistManager::UpdateObject(uint32_t id, uint32_t timestamp, Buffer* buffer) {
	ObjectMap::iterator i = NetObjects.find(id);
	if( i != NetObjects.end() ) {
		// We should never receive updates from server of our own objects
		assert( ! CheckClientId( i->second->objectId, interface->getId() ) );

		LOG_SYNC("UpdateObject(%x) timestamp = %d\n", id, timestamp );
		
		if( i->second->lastTimestamp <= timestamp || 1 ) { // FIXME this disables the check!!  Should be it here at all?
			Attributes* attr = i->second->DNET_GetAttributes();
			Attributes::iterator foundAttr;
			uint8_t	attrId;
 			
			do {
				*buffer >> attrId;
				foundAttr = std::find_if(attr->begin(), attr->end(), std::bind2nd(AttributeFinderByID(),attrId));

				if( foundAttr != attr->end() ) {
					//LOG_SYNC("Found attribtue - updating it: %s #%d\n", foundAttr->getName(), foundAttr->getID());
					foundAttr->unpack(buffer);
				}
				else {
					LOG_SYNC("Attribute not found. Dumping available attributes:\n");
					for(foundAttr = attr->begin(); foundAttr != attr->end(); ++foundAttr ) {
						LOG_SYNC("  %s #%d\n", foundAttr->getName(), foundAttr->getID());
					}
					assert(0 && "Attribute not found");		// FIXME
				}
			}while( buffer->size() && foundAttr != attr->end() );

			assert(i->second);
			i->second->onRemotelyUpdated();
		}
		else {
			LOG_DIST("UpdateObject() - object %x OOO updated ignored (updated contained %d, we got %d\n", id, timestamp, i->second->lastTimestamp  );
		}
	}
	else {
		LOG_DIST("UpdateObject() - object %x not found\n", id);
	}
}

void DistManager::RegisterClass(const char* name, DNET_ClassDescription::CreatorFunc creator, DNET_ClassDescription::DeleterFunc deleter) {
	ClassFunc func;
	func.creator = creator;
	func.deleter = deleter;
	LOG_DIST("RegisterClass(%s)\n", name);
	NetClasses.insert(std::make_pair( std::string(name), func ));
}

void DistManager::Synchronize() {
	//SynchronizeSimple();
	SynchronizeASBS();
}


struct SynchronizeASBSSorter {
	bool operator() ( Server::ClientCon* const &a, Server::ClientCon* const &b  ) {
		return a->bw < b->bw;
	}
};

void DistManager::SynchronizeASBS() {
	if(! interface ) return;
	if( isServer ) {
		Server* s = reinterpret_cast<Server*>(interface);

		// Convert out idClients to a vector, sort it by bw-usage and then iterate it
		std::vector<Server::ClientCon*>		clientOrder;

		for(Server::IdToClientMap::iterator	 j = s->idClients.begin(); j != s->idClients.end(); ++j ) {
			clientOrder.push_back(&j->second);
		}

		std::sort(clientOrder.begin(), clientOrder.end(), SynchronizeASBSSorter() );

		for(std::vector<Server::ClientCon*>::iterator j = clientOrder.begin(); j!= clientOrder.end(); ++j ) {
			SynchronizeASBSFor((*j)->id, s->getENetHost(),(*j)->peer);
		}
	}
	else {
		Client* c = reinterpret_cast<Client*>(interface);
		assert(c);
		SynchronizeASBSFor(0, c->getENetHost(), c->getENetPeer() );
	}
}

bool DistManager::DoSync(uint32_t id, uint8_t forClientId) {
	return (isServer && !CheckClientId( id, forClientId) ||
				(!isServer && CheckClientId( id, interface->getId())));
}

// Helper for SynchronizeASBSFor()
struct AttributesOwner {
		int				pri;
		Attributes	attrs;
		NetObject*	owner;
};

void DistManager::SynchronizeASBSFor(uint32_t forId, ENetHost* enetHost, ENetPeer* enetPeer) {
	float			loisum		= 0;												// Used for normalizing LOI
	int				sparebw		= enetPeer->packetThrottle;		// TODO Reserve some bw for reliable
	uint32_t		ticks		= dnettime();								// Get time
	int				usedbw = 0;

	typedef std::map<uint32_t, int> AvailBWMap;
	AvailBWMap	availbw;

	typedef std::map<uint32_t, NetObjectDataPacket>	DataPackets;
	DataPackets		dataPackets;

	// Synchronize objects that needs fullsync
	for(ObjectMap::iterator i = DistManager::NetObjects.begin(); i != DistManager::NetObjects.end(); ++i ) {
		NetObject*							nObj		= i->second;
		DNET_ClassDescription*	desc		= nObj->DNET_GetClassDescription();
		
		if( DoSync( nObj->objectId, forId) ) {
			if( (ticks - nObj->lastFullSynch) >= desc->getMaxInterval()) {
				nObj->lastFullSynch = nObj->lastSynch = ticks;

				NetObjectDataPacket			data;
				data.setObjectId(nObj->objectId);
				
				Attributes* attr = nObj->DNET_GetAttributes();

				for(Attributes::iterator a  = attr->begin(); a != attr->end(); ++a ) {
					uint8_t attrId = a->getID();
					data.buffer << attrId;
					a->pack(&data.buffer);
				}
				if( isServer ) {
					Server* s = reinterpret_cast<Server*>(interface);
					usedbw += s->send(forId,&data);
				}
				else {
					assert(forId == 0);
					Client* c = reinterpret_cast<Client*>(interface);
					c->send(&data);
				}
				LOG_SYNC("Send Full NetObjectDataPacket (%x) with timestamp = %d to client-%d\n", data.getObjectId(), data.getTimestamp(), forId );
			}
		}
	}

	// Check bandwidth
	uint32_t elapsed = ticks - enetHost->bandwidthThrottleEpoch;
	if( elapsed == 0 )	{
		// FIXME Perhaps we should just use the last calculated inbw/outbw
		return;
	}

	uint32_t outbw	= enetPeer->outgoingDataTotal / elapsed;
		
	sparebw = enetPeer->packetThrottle - outbw;			// KBytes
	sparebw *= 1024;									// Bytes..

	// Normalize Level Of Interest
	for(ObjectMap::iterator i = DistManager::NetObjects.begin(); i != DistManager::NetObjects.end(); ++i ) {
		NetObject*	nObj = i->second;

		if( DoSync( nObj->objectId, forId) )
			loisum += GetLOI(forId, nObj->objectId);
	}

	if( loisum == 0 ) {
		//LOG_SYNC("LOISUM is zero, stopping sync\n");
		return;
	}

	LOG_SYNC("loisum = %f\n", loisum );

	// Distribute the bandwidth
	for(ObjectMap::iterator i = DistManager::NetObjects.begin(); i != DistManager::NetObjects.end(); ++i ) {
		NetObject*	nObj = i->second;
		
		if( DoSync( nObj->objectId, forId) ) {
			double v = sparebw *  GetLOI(forId, nObj->objectId) / loisum;
			availbw[nObj->objectId] = static_cast<uint32_t>(floor(v));
		}
	}

	// Collect all Attributes (that needs to be considered for a partial update) into a subgke list, sorted by priority
	typedef std::list<AttributesOwner> AOList;
	AOList list;
	{
		int pri;
		AttributesOwner	ao;
		
		for(ObjectMap::iterator i = NetObjects.begin(); i != NetObjects.end(); ++i ) {
			NetObject*							nObj		= i->second;
			DNET_ClassDescription*	desc		= nObj->DNET_GetClassDescription();
				
			if( DoSync( nObj->objectId, forId) ) {
				if( (ticks - nObj->lastSynch) >= desc->getMinInterval() ) {
					nObj->lastSynch = ticks;
					
					Attributes* attr = nObj->DNET_GetAttributes();

					ao.owner = nObj;

					pri = -1;

					for(Attributes::iterator a  = attr->begin(); a != attr->end(); ++a ) {
						if( a->getPriority() != pri ) {
							if( pri != -1 ) {
								ao.pri = pri;
								list.push_back(ao);
								//LOG_SYNC("dump: Adding Attribute Unit: id = %x, pri = %d, owner = %x, attrs = %d\n",
								//	ao.owner->getObjectId(), ao.pri, (unsigned int)ao.owner, ao.attrs.size() );
								ao.attrs.clear();
							}
							pri = a->getPriority();
						}
						ao.attrs.push_back(*a);
					}
					// The attribute in the loop isnt added, so do it manually here
					if( attr->size() ) {
						ao.pri = pri;
						list.push_back(ao);
						//LOG_SYNC("dump: Adding Attribute Unit: id = %x, pri = %d, owner = %x, attrs = %d\n",
						//	ao.owner->getObjectId(), ao.pri, (unsigned int)ao.owner, ao.attrs.size() );
						ao.attrs.clear();
					}
				}
				else {
					//LOG_SYNC("Synchronize is less than min interval. Need to wait. (%d < %d)\n", (ticks - nObj->lastSynch), desc->getMinInterval() );
				}
			}
		}
	}

	// .. Begin packing
	DataPackets::iterator		dPacket;
	for(AOList::iterator a  = list.begin(); a != list.end(); ++a ) {
		//LOG_SYNC("dump: AttributeUnit Owner = %x, Priority = %d\n", (unsigned int)a->owner, a->pri );
		NetObject*		nObj= a->owner;
		dPacket = 		dataPackets.find(nObj->objectId);

		if( dPacket == dataPackets.end() ) {
			// Not found - add it
			NetObjectDataPacket	data;
			data.setObjectId(nObj->objectId);
			dataPackets[nObj->objectId] = data;

			dPacket = dataPackets.find(nObj->objectId);
			assert(dPacket != dataPackets.end());
		}

 		Buffer	attrBuf;
		for(Attributes::iterator b  = a->attrs.begin(); b != a->attrs.end(); ++b ) {
			uint8_t attrId = b->getID();
			attrBuf<< attrId;
			b->pack(&attrBuf);
		}

		if( attrBuf.size() <= availbw[nObj->objectId] ) {
			availbw[nObj->objectId] -= attrBuf.size();
			dPacket->second.buffer.moveFrom( attrBuf );
			LOG_SYNC("  Added AttributeUnit for NetObject-%x pri %d  datapacket size %d\n", nObj->objectId, a->pri, dPacket->second.buffer.size() );
		}
	}

	for( DataPackets::iterator it = dataPackets.begin(); it != dataPackets.end(); ++it ) {
		// No reason to send nothing
		if( it->second.buffer.size() == 0 ) {
			continue;
		}

		LOG_SYNC("Send Partial NetObjectDataPacket %x with timestamp = %d for client %d\n", it->second.getObjectId(), it->second.getTimestamp(), forId );

		if( isServer ) {
			Server* s = reinterpret_cast<Server*>(interface);
			usedbw += s->send(forId,&it->second);
		}
		else {
			assert(forId == 0);
			Client* c = reinterpret_cast<Client*>(interface);
			c->send(&it->second);
		}	
	}

	if( isServer ) {
		Server* s = reinterpret_cast<Server*>(interface);
		s->idClients[forId].bw = usedbw;
	}
}

#include <stdlib.h> // Only for rand() below
void DistManager::SynchronizeSimple() {
	if(! interface ) return;
	
	DNET_ClassDescription*	desc;
	NetObject* nObj;

	LOG_DIST("Got %d NetObjects\n", NetObjects.size());

	struct timeval now;
	gettimeofday(&now, NULL);
	uint32_t ticks=(now.tv_sec-interface->startTime.tv_sec)*1000+(now.tv_usec-interface->startTime.tv_usec)/1000;

	bool fullSync			= false;
	bool partialSync		= false;
	
	for(ObjectMap::iterator i = DistManager::NetObjects.begin(); i != DistManager::NetObjects.end(); ++i ) {
		fullSync			= false;
		partialSync		= false;
		ENetHost	*		enetHost;
		ENetPeer*		enetPeer;
	
		nObj = i->second;
		desc = nObj->DNET_GetClassDescription();

		fullSync		= (ticks - nObj->lastFullSynch) >= desc->getMaxInterval();
		partialSync	= (ticks - nObj->lastSynch) >= desc->getMinInterval();

		if( fullSync || partialSync ) {
			nObj->lastSynch = ticks;
			if( fullSync ) 
				nObj->lastFullSynch = ticks;
			
			/*
			if( fullSync ) {
				LOG_SYNC("Full Synchronization\n");
			}
			else {
				LOG_SYNC("Partial Synchronization\n");
			}
			*/

			if( isServer ) {
				// Foreach client, but do not send object back to client
				Server* s = reinterpret_cast<Server*>(interface);
				if( fullSync )
					s->updateLOI();

				uint32_t now = enet_time_get ();
				
				for(Server::IdToClientMap::iterator	 j = s->idClients.begin(); j != s->idClients.end(); ++j ) {
					enetHost = s->getENetHost();
					enetPeer = j->second.peer;

					// Silly LOI use
					if( (GetLOI(j->first, nObj->objectId) * 10)  > rand()%10 ) {
 						LOG_SYNC("Skipping object - LOI too low %f\n", GetLOI(j->first, nObj->objectId) );
						continue;
					}
					
					uint32_t elapsed = now - enetHost->bandwidthThrottleEpoch;
					
					if( elapsed == 0 )	{
						// FIXME Perhaps we should just use the last calculated inbw/outbw
						continue;
					}
					
					uint32_t inbw		= enetPeer->outgoingDataTotal / elapsed;
					uint32_t outbw	= enetPeer->incomingDataTotal / elapsed;
					
					LOG_SYNC("BW client %d: in = %d, out = %d. Total: in = %d, out = %d, time = %d\n",j->first, inbw, outbw,
						enetPeer->incomingDataTotal, enetPeer->outgoingDataTotal, elapsed);
					LOG_SYNC("BW: packetThrottle =%d, packetThrottleLimit = %d, windowSize = %d\n", enetPeer->packetThrottle, enetPeer->packetThrottleLimit,
						enetPeer->windowSize );
					
					if(! CheckClientId( nObj->objectId, j->first ) )  {

						NetObjectDataPacket	data;
						data.setObjectId(nObj->objectId);

						Attributes* attr = nObj->DNET_GetAttributes();

						int lastPri = -1;
						for(Attributes::iterator a  = attr->begin(); a != attr->end(); ++a ) {
							if( fullSync || lastPri == -1 || (lastPri != -1 && a->getPriority() == lastPri ) ) {
								LOG_SYNC(" packing %x  '%s' with priority %d\n", nObj->objectId, a->getName(), a->getPriority());
								uint8_t attrId = a->getID();
								data.buffer << attrId;
								a->pack(&data.buffer);
								lastPri = a->getPriority();
							}
						}
						LOG_SYNC(" Sync client %d with %s#%x - it had %d bytes of buffer\n", j->first, desc->getName(), nObj->objectId, data.buffer.size() );
						s->send(j->first,&data);
					}
					else {
						LOG_SYNC(" Client %d owns %s#%x it. No sync\n", j->first, desc->getName(), nObj->objectId );
					}
				}
			}
			else {
				Client* c = reinterpret_cast<Client*>(interface);
				enetHost = c->getENetHost();
				enetPeer = c->getENetPeer();
				
				uint32_t now = enet_time_get ();
				uint32_t elapsed = now - enetHost->bandwidthThrottleEpoch;
				
				if( elapsed == 0 )	{
					// FIXME Perhaps we should just use the last calculated inbw/outbw
					continue;
				}
				uint32_t inbw = enetPeer->outgoingDataTotal / elapsed;
				uint32_t outbw = enetPeer->incomingDataTotal / elapsed;
				
				LOG_SYNC("BW server: in = %d, out = %d. Total: in = %d, out = %d, time = %d\n",inbw, outbw,
					enetPeer->incomingDataTotal, enetPeer->outgoingDataTotal, elapsed);
				LOG_SYNC("BW: packetThrottle =%d, packetThrottleLimit = %d, windowSize = %d\n", enetPeer->packetThrottle, enetPeer->packetThrottleLimit,
					enetPeer->windowSize );

				// Only synchronize own objects
				if( CheckClientId( nObj->objectId, interface->getId() ) )  {
					LOG_SYNC(" Sync %s#%x to server\n", desc->getName(), nObj->objectId );

					NetObjectDataPacket	data;
					data.setObjectId(nObj->objectId);
					Attributes* attr = nObj->DNET_GetAttributes();
	
					LOG_SYNC("DNET_GetAttributes = %x\n", (unsigned int)attr);
					int lastPri = -1;
					for(Attributes::iterator a  = attr->begin(); a != attr->end(); ++a ) {
						if( fullSync || lastPri == -1 || (lastPri != -1 && a->getPriority() == lastPri) ) {
							LOG_SYNC("  Synching(pack) (%x) attribute '%s' (attrid = %d ~ %d) with priority %d\n", nObj->objectId, a->getName(), a->getID(), 1 << a->getID(), a->getPriority() );
							uint8_t attrId = a->getID();
							data.buffer << attrId;
							a->pack(&data.buffer);
							lastPri = a->getPriority();
						}
					}
					LOG_SYNC(" NetObjectDataPacket payload: %d bytes\n", data.buffer.size() );
					interface->send(&data);
				}
				else {
					LOG_SYNC("No sync. I am client-%d and object got id %x\n", interface->getId(), nObj->objectId );
				}
			}
		}
	}
}

void DistManager::Add(NetObject& t) {
	t.objectId = GenId();
	LOG_DIST("Adding: %s#%x\n", t.DNET_GetClassDescription()->getName(), t.objectId);
	CreateObject(&t);
	SendCreationPacket(t.objectId,*t.DNET_GetClassDescription());
}

void DistManager::Remove(NetObject& t) {
//	LOG_DIST("Removing: %s#%x\n", t.DNET_GetClassDescription()->getName(), t.objectId );

	SendDeletionPacket(t.objectId, *t.DNET_GetClassDescription());
	DeleteObject(t.objectId);
}

void DistManager::SendAllNetObjects(Server* s,uint8_t clientId) {
	LOG_DIST("SendAllNetObjects() to client %d\n", clientId );
	DNET_ClassDescription*	desc;
	NetObjectCreatePacket	create;
	NetObject* nObj;
	
	for(ObjectMap::iterator i = NetObjects.begin(); i != NetObjects.end(); ++i ) {
		nObj = i->second; 
		desc = nObj->DNET_GetClassDescription();
		
		LOG_DIST("CreateObject(%s#%x) [brining client up to speed]\n", desc->getName(), nObj->objectId );
		create.setClass(desc->getName());
		create.setObjectId(nObj->objectId);
		s->sendReliable(clientId,&create);
	}
}

bool DistManager::HandleServerPacket(Server* s,Packet *packet, uint8_t senderId) {
	if( PacketParser::InternalPacket(packet) ) {
		if( NetObjectCreatePacket* p = dynamic_cast<NetObjectCreatePacket*>(packet) ) {
			// Validating ID
			if( DistManager::CheckClientId(p->getObjectId(),senderId) ) {
				LOG_DIST("Creating object: '%s' %x\n", p->getClass(), p->getObjectId());
				NetObject* nObj = dnet::DistManager::CreateObject(p->getClass(), p->getObjectId(), false);

				if( nObj == 0L ) {
					// FIXME What to do?
					LOG_DIST("** WARNING, Unknown NetObject: %s#%d\n", p->getClass(), p->getObjectId());
				}
				else {
					// FIXME dnet::DistManager::UpdateObject(p->getObjectId(), 0,  &p->buffer );
					
					// Distribute the NetObjectCreatePacket to everybody except c.id
					for(Server::IdToClientMap::iterator	 i = s->idClients.begin(); i != s->idClients.end(); ++i ) {
						if( i->first != senderId )
							s->sendReliable(  i->first, p );
					}
				}
			}
			else {
				LOG_DIST("** BOGUS ID (%s#%x) (HandleServerPacket got NetObjectCreatePacket) Sender is %x**\n", p->getClass(), p->getObjectId(), senderId);
			}
			return true;
		}
		else if( NetObjectDeletePacket* p = dynamic_cast<NetObjectDeletePacket*>(packet) ) {
			// Validating ID
			if( DistManager::CheckClientId(p->getObjectId(),senderId) ) {
				LOG_DIST("Deleting object: %x\n", p->getObjectId());

				dnet::DistManager::DeleteObject(p->getObjectId());

				// Distribute the NetObjectCreatePacket to everybody except c.id
				for(Server::IdToClientMap::iterator	 i = s->idClients.begin(); i != s->idClients.end(); ++i ) {
					if( i->first != senderId )
						s->sendReliable(  i->first, p );
				}
			}
			else {
				LOG_DIST("** BOGUS ID (%x) (HandleServerPacket got NetObjectDeletePacket) Sender is %d **\n", p->getObjectId(), senderId);
			}
		}
		else if( NetObjectDataPacket* p = dynamic_cast<NetObjectDataPacket*>(packet) ) {
			if( DistManager::CheckClientId(p->getObjectId(),senderId) ) {
				dnet::DistManager::UpdateObject(p->getObjectId(), p->getTimestamp(), &p->buffer );
				// No, we dont distribute it here - this will be done in Synchronize()
			}
			else {
				LOG_DIST("** BOGUS ID (%x)  (HandleServerPacket got NetObjectDataPacket) Sender is %d**\n", p->getObjectId(), senderId);
			}
		}
		else {
			assert(0 && "Unknown internal packet");
		}

		return true;
	}
	else {
		return false;
	}
}

bool DistManager::HandleClientPacket(Client*,Packet *packet) {
	if( PacketParser::InternalPacket(packet) ) {
		// Create NetObject
		if( NetObjectCreatePacket* p = dynamic_cast<NetObjectCreatePacket*>(packet) ) {
			NetObject* nObj = dnet::DistManager::CreateObject(p->getClass(), p->getObjectId(), false);
			if( nObj == 0L ) {
				// FIXME What to do?
				LOG_DIST("** WARNING, Unknown NetObject: %s#%d\n", p->getClass(), p->getObjectId());
			}
			else {
				// FIXME dnet::DistManager::UpdateObject(p->getObjectId(), 0,  &p->buffer );
			}
		}
		// Delete NetObject
		else if( NetObjectDeletePacket* p = dynamic_cast<NetObjectDeletePacket*>(packet) ) {
			dnet::DistManager::DeleteObject(p->getObjectId());
		}
		// Update NetObject
		else if( NetObjectDataPacket* p = dynamic_cast<NetObjectDataPacket*>(packet) ) {
			dnet::DistManager::UpdateObject(p->getObjectId(), p->getTimestamp(), &p->buffer );
		}
		else {
			assert( 0 && "Handle internal packet here");
		}

		return true;
	}
	else {
		return false;
	}
}

// Bad name really, Should be called reaper/garbage collector, as clientId is already gone.
void DistManager::ClientDisconnected(uint8_t clientId) {
	if(! interface ) return;
	assert(isServer);
	
	NetObjectDeletePacket	die;

	LOG_DIST("Checking %d NetObjects.\n", NetObjects.size() );
	
	for(ObjectMap::iterator i = NetObjects.begin(); i != NetObjects.end(); ++i ) {
		LOG_DIST("Check if %x needs to be removed, as %d is gone\n", i->first, clientId );
		if(! i->second ) {
			LOG_DIST("**HACK** i->second == NULL - ignored\n" );
			continue;
		}
		if( CheckClientId(i->second->objectId,clientId) ) {
		
			uint32_t objectId = i->second->objectId;
			
			// Make sure everyone else gets the message
			die.setObjectId(objectId);
			interface->sendReliable(&die);

			// Perform the operation
			DeleteObject(objectId);

			// Now, iterators are broken - restart
			i = NetObjects.begin();
		}
	}
}

void DistManager::SetLOI(uint8_t clientId, uint32_t objectId, float value) {
	if( CheckClientId(objectId,clientId) ) return;
	assert(value >= 0 );
	assert(value <= 1 );
	NetObjectsLOI[clientId][objectId] = value;
}

float DistManager::GetLOI(uint8_t clientId, uint32_t objectId) {
	//return isServer ? NetObjectsLOI[clientId][objectId] : 1 ;
	return 1;
}

} // namespace dnet
