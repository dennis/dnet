bin_PROGRAMS = server commander client

server_SOURCES = server.cpp
server_LDFLAGS=-lSDL_gfx $(top_srcdir)/src/*.o $(top_srcdir)/enet/*.o

client_SOURCES = client.cpp history.cpp
client_LDFLAGS=-lSDL_gfx $(top_srcdir)/src/*.o $(top_srcdir)/enet/*.o

commander_SOURCES = commander.cpp
commander_LDFLAGS=-lSDL_gfx $(top_srcdir)/src/*.o $(top_srcdir)/enet/*.o

AM_CPPFLAGS=-I$(top_srcdir)/src/include/  -I$(top_srcdir)/enet/include/
