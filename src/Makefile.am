## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(SNAKES_CFLAGS) \
	-I$(JSON11_SRCDIR) \
	-std=c++11

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = snakes

snakes_SOURCES = \
	main.cpp \
	snake.cpp \
	snake.h \
	sfml.cpp \
	sfml.h \
	$(JSON11_SRCDIR)/json11.cpp

snakes_LDFLAGS = 

snakes_LDADD = $(SNAKES_LIBS)

