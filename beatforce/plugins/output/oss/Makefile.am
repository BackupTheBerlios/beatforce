ossltlibs = liboss.la
ossldflags = 

lib_LTLIBRARIES = $(ossltlibs) 

libdir = @PLUGIN_DIR@/output

liboss_la_LDFLAGS = -avoid-version -module -export-dynamic -Wall
liboss_la_SOURCES = oss.c 
INCLUDES = -I$(top_builddir)/src -I/usr/local/include


