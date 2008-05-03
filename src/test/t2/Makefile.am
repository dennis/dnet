bin_PROGRAMS = test

test_SOURCES = test.cpp
test_LDFLAGS = -lSDL_gfx $(top_srcdir)/src/*.o $(top_srcdir)/enet/*.o
AM_CPPFLAGS = -I$(top_srcdir)/src/include/ -I$(top_srcdir)/enet/include/ $(SDL_CFLAGS) -DDEBUG
