lib_LTLIBRARIES = libbeatforceogg.la

libdir = @PLUGIN_DIR@/input

libbeatforceogg_la_LDFLAGS = -module -avoid-version

libbeatforceogg_la_SOURCES = ogg.c ogg.h 
INCLUDES =  -I../../../src -I$(top_builddir)/src $(MADDIR)
LIBS = -lvorbis -lvorbisfile
