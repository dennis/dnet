#ifndef COMMON_H
#define COMMON_H
#define WIDTH 800 //640
#define HEIGHT 600 //480
#define TICK_INTERVAL 50

#include <dnet/dnet.hpp>
#include <vector>
#include <algorithm>
#include "common.h"

class Spaceship;
class Shot;

typedef std::vector<Spaceship*>	SpaceshipsVector_t;
SpaceshipsVector_t		spaceships;

typedef std::vector<Shot*>	ShotsVector_t;
ShotsVector_t	shots;

class Spaceship : public dnet::NetObject {
public:
	float	x, y;
	float	oldX, oldY;
	int		radius;
	float	speed;
	bool	lineMovement;
	bool	player;

	Spaceship(int _x=0, int _y=0) : x(_x), y(_y), oldX(_x), oldY(x), radius(3), speed(1), lineMovement(false), player(false) {
	}

	void onRemotelyCreated() {
		spaceships.push_back(this);
		printf("onRemotelyCreated(%x) %x: Spaceship count %d: ", 
			getObjectId(), (unsigned int)this, spaceships.size() );
		debug();
	}

	void onRemotelyDeleted() {
		SpaceshipsVector_t::iterator i = find(spaceships.begin(), spaceships.end(), this);
		if( i != spaceships.end() )
			spaceships.erase(i);
		printf("onRemotelyDeleted(%x) %x: Spaceship count %d\n", 
			getObjectId(), (unsigned int)this, spaceships.size() );
	}

	void onRemotelyUpdated() {
//		printf("onRemotelyUpdated(%x) %x: ", getObjectId(), (unsigned int)this );
//		debug();
	}

	void debug() {
		printf("Spaceship#%x %x updated spaceship: pos=%f,%f, r=%d\n", getObjectId(), (unsigned int)this, x, y, radius );
	}

	DNET_DESC(Spaceship,
		DNET_ATTRIBUTE(x,30);
		DNET_ATTRIBUTE(y,30);
		DNET_ATTRIBUTE(oldX,20);
		DNET_ATTRIBUTE(oldY,20);
		DNET_ATTRIBUTE(radius,10);
		DNET_ATTRIBUTE(speed,10);
	);

};

DNET_REGISTER(Spaceship,10,50);

class Shot : public dnet::NetObject {
public:
	float	posX, posY;
	float	dirX, dirY;

	Shot(float x=0, float y=0, float dx=0, float dy=0) : posX(x), posY(y), dirX(dx), dirY(dy) {
	}

	void onRemotelyCreated() {
		shots.push_back(this);
		printf("onRemotelyCreated(%x) %x. Shot count %d\n", 
			getObjectId(), (unsigned int)this, shots.size());
		onRemotelyUpdated();
	}

	void onRemotelyDeleted() {
		ShotsVector_t::iterator i = find(shots.begin(), shots.end(), this);
		if( i != shots.end() )
			shots.erase(i);
		printf("onRemotelyDeleted(%x) %x Shot count %d\n",
			getObjectId(), (unsigned int)this, shots.size() );
	}
	void onRemotelyUpdated() {
//		printf("onRemotelyUpdated(%x) %x updated shot: pos=%f,%f dir=%f,%f\n", 
//			getObjectId(), (unsigned int)this, posX, posY, dirX, dirY );
	}

	DNET_DESC(Shot,
		DNET_ATTRIBUTE(dirX,20);
		DNET_ATTRIBUTE(dirY,20);
		DNET_ATTRIBUTE(posX,20);
		DNET_ATTRIBUTE(posY,20);
	);
};

DNET_REGISTER(Shot,10,50);

#endif
