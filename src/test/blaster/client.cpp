/*
Blaster
=====
./server
./client random				[RandomController,NullGUI]
./client [-window] observer [NullController,GUI]

TODO?
./client formation 10		[FormationController?,NullGUI]
./client [-window] player	[PlayerController,GUI] ?
*/

#include <assert.h>
#include <math.h>
#include <memory>
#include <string.h>

#include <SDL.h>

#include "GUI.h"
#include "Controller.h"

static Uint32 nextTime;
static GUI*	gui;
static Controller* controller;

Uint32 timeLeft(void) {
    Uint32 now;
    now = SDL_GetTicks();
    if(nextTime <= now)
        return 0;
    else
        return nextTime - now;
}

void waitFrame() {
	SDL_Delay(timeLeft());
	nextTime += TICK_INTERVAL;
}

void gameLoop(dnet::Client &client) {
	bool done = false;
	SDL_Event	event;

	while(!done) {
		// Handle Events
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_QUIT:
				done = true;
				break;
			case SDL_KEYDOWN:
				done = event.key.keysym.sym == SDLK_ESCAPE;
				break;
			}
		}

		// Move spaceships
		controller->render(spaceships);

		// Draw stuff
		gui->render(spaceships,shots);
		
		// Wait
		waitFrame();

		client.update();
	}

	client.disconnect();
}

void usage(char* me) {
	printf("Usage: %s <fullscreen|window|null> <random|observer>\n", me);
}

int main(int argc, char* argv[]) {
	int j = 0;
	for( int i=1; i < argc; i++ ) {
		if( j == 0 ) { // GUI
			if( gui ) {
				fprintf(stderr, "GUI already configured!\n");
				usage(argv[0]);
				return 1;
			}
			if( !strcmp(argv[i], "fullscreen" ) )
				gui = new FullscreenGUI();
			else if( !strcmp(argv[i], "window" ) )
				gui = new WindowedGUI();
			else if( !strcmp(argv[i], "null" ) )
				gui = new NullGUI();
			else {
				fprintf(stderr, "Invalid GUI: '%s'\n", argv[i] );
				usage(argv[0]);
				return 1;
			}
			j++;
		}
		else if( j == 1 ) { // Controller
			if( controller ) {
				fprintf(stderr, "Controller already configured!\n");
				usage(argv[0]);
				return 1;
			}
			if( !strcmp(argv[i],"random") )
				controller = new RandomController();
			else if( !strcmp(argv[i],"observer") )
				controller = new Controller();
			else {
				fprintf(stderr, "Invalid Controller: '%s'\n", argv[i] );
				usage(argv[0]);
				return 1;
			}
			j++;
		}
		else {
			usage(argv[0]);
			return 1;
		}
	}

	if( j != 2 ) {
		usage(argv[0]);
		return 1;
	}

	if( dnet::init() ) {

		assert(gui);
		assert(controller);

		dnet::Client	client;
		dnet::DistManager::SetOwner(client);

		if(! client.connect("localhost",2000) ) {
			puts("Failed connecting to server");
			return 1;
		}

		gui->init();
		controller->init(spaceships);

		gameLoop(client);

	}
	else {
		puts("DNET initialization failed");
	}
}
