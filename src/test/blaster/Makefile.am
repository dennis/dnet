bin_PROGRAMS = server client

server_SOURCES = server.cpp
server_LDFLAGS = -lSDL_gfx $(top_srcdir)/src/*.o $(top_srcdir)/enet/*.o

client_SOURCES = client.cpp
client_LDFLAGS = -lSDL_gfx $(top_srcdir)/src/*.o $(top_srcdir)/enet/*.o

AM_CPPFLAGS = -I$(top_srcdir)/src/include/ -I$(top_srcdir)/enet/include/ $(SDL_CFLAGS)

#client_LDFLAGS=$(top_srcdir)/enet/*.o $(SDL_LIBS) -L$(top_srcdir)/src/ -ldnet
#server_LDFLAGS=$(top_srcdir)/enet/*.o $(SDL_LIBS) -L$(top_srcdir)/src/ -ldnet


# works: g++ -lSDL -lSDL_gfx client.o -o client -L../../ -ldnet ../../../enet/*.o

#AM_CPPFLAGS = \
#	-I$(top_srcdir)/src/include/ \
#	-I$(top_srcdir)/enet/include/ \
#	$(SDL_CFLAGS)

#ENET_LDFLAGS=$(HOME)/dev/enet/*.o
#ENET_CFLAGS=-I$(HOME)/dev/enet/include/
#
#DNET_LDFLAGS=$(ENET_LDFLAGS) -L../.. -ldnet
#DNET_CFLAGS=$(ENET_CFLAGS) -I../../include/
#
#SDL_LDFLAGS=`sdl-config --libs` -lSDL_gfx
#SDL_CFLAGS=`sdl-config --cflags`
#
#CXXFLAGS = -Wall -Wunused -Wcast-qual -Wconversion -Woverloaded-virtual -Winline -Werror -ggdb  $(DNET_CFLAGS) $(SDL_CFLAGS) -DDEBUG
#LDFLAGS=$(DNET_LDFLAGS) $(SDL_LDFLAGS)
#
