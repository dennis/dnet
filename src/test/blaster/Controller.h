#ifndef CONTROLLER_H
#define CONTROLLER_H

typedef std::vector<Spaceship*>	SpaceshipsVector_t;

class Controller {
public:
	virtual bool init(SpaceshipsVector_t&) { return true; }
	virtual void render(SpaceshipsVector_t&) {};
	virtual ~Controller() {
	};
};

class RandomController : public Controller{
public:
	bool init(SpaceshipsVector_t& spaceships) {
		Spaceship* ss;
		for(int j = 0; j < 1; j++ ) {
			ss = new Spaceship(rand() % WIDTH, rand() % HEIGHT);
			ss->player = (j == 0);
			dnet::DistManager::Add(*ss);
			spaceships.push_back(ss);
		}

		return true;
	}
	void render(SpaceshipsVector_t& spaceships) {
		// Move ships
		Spaceship* sp;
		for( SpaceshipsVector_t::iterator i = spaceships.begin(); i != spaceships.end(); ++i ) {
			sp = *i;

			if( ! sp->isLocal() )
				continue;

			float dx = sp->x - sp->oldX;
			float dy = sp->y - sp->oldY;

			if( sp->lineMovement ) {
				if( dx > 3 )
					dx = 3;
				if( dx < -3 )
					dx = -3;
				if( dy > 3 )
					dy = 3;
				if( dy < -3 )
					dy = -3;
			}
			else {
				int r = rand()%3;
				if( r == 0 ) 
					dx += (rand()%7)+1;
				if( r == 1 ) 
					dx -= (rand()%7)+1;
				if( r == 2 ) 
					dx = (-1)*dx;

				r = rand()%3;
				if( r == 0 )
					dy +=  ((rand()%7)+1);
				if( r == 1 ) 
					dy -= ((rand()%7)+1);
				if( r == 2 )
					dy -= (-1)*dy;

			}
			sp->oldX = sp->x;
			sp->oldY = sp->y;

			sp->x += (dx * sp->speed);
			sp->y += (dy * sp->speed);

			// Bounds corrections
			if( sp->x >= WIDTH - sp->radius )
				sp->x = WIDTH - sp->radius;
			else if( sp->x < 0 )
				sp->x = sp->radius;

			if( sp->y >= HEIGHT - sp->radius )
				sp->y = HEIGHT - sp->radius;
			else if( sp->y < 0 )
				sp->y = sp->radius;

			sp->lineMovement = ! (rand()%6 == 0);

			// Shoot?
			if( rand()%30 == 0 ) {
				Shot*	shot;
				int dirX = ((rand()%3) - 1) * 3;
				int dirY = ((rand()%3) - 1) * 3;

				if( dirX || dirY ) {
					// Make sure shots are moving
					shots.push_back(shot = new Shot(sp->x, sp->y, dirX, dirY));
					dnet::DistManager::Add(*shot);
				}
			}
		}

		Shot*	shot;
		for( ShotsVector_t::iterator i = shots.begin(); i != shots.end(); ++i ) {
			shot = *i;
			if( ! shot->isLocal() )
				continue;
			shot->posX += shot->dirX;
			shot->posY += shot->dirY;
		}

		// Cleanup shots which is out of bounds
		for( ShotsVector_t::iterator i = shots.begin(); i != shots.end(); ++i ) {
			shot = *i;
			if( ! shot->isLocal() )
				continue;
			if( shot->posX < 0 || shot->posX > WIDTH || shot->posY < 0 || shot->posY > HEIGHT ) {
				shots.erase(i);
				dnet::DistManager::Remove(*shot);
				delete shot; 
				return;		// HACK
			}
		} 
	}

	~RandomController() {
	}
};
#endif
