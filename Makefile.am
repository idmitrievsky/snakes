## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src 

snakesdocdir = ${prefix}/doc/snakes
snakesdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS


EXTRA_DIST = $(snakesdoc_DATA)


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(snakesdocdir)
