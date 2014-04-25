## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(SNAKES_CFLAGS) \
	-std=c++0x

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = snakes

graphcut_SOURCES = \
	main.cpp \
	snakes.cpp \
	snakes.h \
	sfml.cpp \
	sfml.h

snakes_LDFLAGS = 

snakes_LDADD = $(SNAKES_LIBS)

