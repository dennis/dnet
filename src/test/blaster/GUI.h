#ifndef GUI_H
#define GUI_H
#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#include "common.h"

class GUI {
public:
	virtual bool init() {
		return true;
	}
	virtual void render(SpaceshipsVector_t, ShotsVector_t) {
	}
	virtual ~GUI() {
	}
};

class NullGUI : public GUI {
public:
	virtual bool init() {
		return true;
	}
	virtual void render(SpaceshipsVector_t, ShotsVector_t) {
	}
	virtual ~NullGUI() {
	}
};

#define NUM_STARS	512
#define STAR_MOVE_X 3
#define STAR_MOVE_Y 1

class FullscreenGUI : public GUI {
private:
	SDL_Surface*	 screen;		// This is not explicit free'd, as SQL_Quit do this
	
	int starX[NUM_STARS];
	int starY[NUM_STARS];
	int starSpeed[NUM_STARS];
	int starColorR[NUM_STARS];
	int starColorG[NUM_STARS];
	int starColorB[NUM_STARS];

	bool windowed;

public:
	FullscreenGUI(bool w=false) : windowed(w) {
	}

	bool init() {
		if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
			fprintf(stderr,"SDL_Init: %s\n", SDL_GetError() );
			return false;
		}
		atexit(SDL_Quit);

		screen =  SDL_SetVideoMode(WIDTH,HEIGHT, 0, SDL_HWPALETTE | SDL_HWSURFACE | SDL_DOUBLEBUF | (windowed? 0 : SDL_FULLSCREEN));
			
		if(! screen ) {
			fprintf(stderr,"SDL_SetVideoMode: %s\n", SDL_GetError() );
			return false;
		}
		initStars();
		return true;
	}

	void initStars() {
		for(int i = 0; i < NUM_STARS; i++ ) {
			starX[i] = rand() %  WIDTH;
			starY[i] = rand() % HEIGHT;
			starColorR[i] = starColorG[i] = starColorB[i] = 0;
			switch( rand()%10 ) {
				case 0:
				case 1:
				case 2:
					starColorB[i] = 200;
					starSpeed[i] = 1;
					break;
				case 3:
				case 4:
					starColorR[i] = 200;
					starSpeed[i] = 1;
					break;
				case 5:
				case 6:
					starColorR[i] = starColorG[i] = starColorB[i] = 50 + (rand() %155);
					starSpeed[i] = 1;
					break;
				default:
					starColorR[i] = starColorG[i] = starColorB[i] = 150 + (rand() %105);
					starSpeed[i] = 2;

			}
		}
	}

	void render(SpaceshipsVector_t _spaceships, ShotsVector_t _shots) {
		// Move stars
		for( int i = 0; i < NUM_STARS; i++ ) {
			starX[i] = (starX[i] + STAR_MOVE_X*starSpeed[i]) % WIDTH;
			starY[i] = (starY[i] + STAR_MOVE_Y*starSpeed[i]) % HEIGHT;
		}

		// Clear
		Uint32 color = SDL_MapRGBA(screen->format, 0,0,0,0);
		SDL_FillRect (screen, NULL, color);

		// Draw Stars
		for(int i = 0; i < NUM_STARS; i++ )
			pixelRGBA(screen, starX[i], starY[i], starColorR[i], starColorG[i], starColorB[i], 255);

		// Draw spaceships
		uint32_t	shipColorLocal		= SDL_MapRGB(screen->format, 0xaa, 0xaa, 0xaa);
		uint32_t	shipColorForeign	= SDL_MapRGB(screen->format, 0xff, 0x00, 0xff);
		uint32_t	shipColorPlayer		= SDL_MapRGB(screen->format, 0xff, 0xff, 0xff);
		uint32_t	shipColor;
		for( SpaceshipsVector_t::iterator i = _spaceships.begin(); i != _spaceships.end(); ++i ) {
			if( (*i)->isLocal() ) 
				shipColor = (*i)->player ? shipColorPlayer : shipColorLocal;
			else
				shipColor = shipColorForeign;
			filledCircleColor(screen, static_cast<int>(roundl((*i)->x)), static_cast<int>(roundl((*i)->y)), (*i)->radius, shipColor);
		}

		// And shots
		uint32_t	shotColor		= SDL_MapRGB(screen->format, 0xff, 0xff, 0xff);
		for( ShotsVector_t::iterator i = _shots.begin(); i != _shots.end(); ++i ) {
			lineColor(screen, 
				static_cast<int>((*i)->posX), static_cast<int>((*i)->posY),
				static_cast<int>((*i)->posX + (*i)->dirX), static_cast<int>((*i)->posY + (*i)->dirY),
				shotColor );
		}

		SDL_Flip(screen);
	}
};

class WindowedGUI : public FullscreenGUI {
public:
	WindowedGUI() : FullscreenGUI(true) {
	}
};
#endif
