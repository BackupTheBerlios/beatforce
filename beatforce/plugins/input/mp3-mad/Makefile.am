lib_LTLIBRARIES = libbeatforcemp3mad.la

libdir = @PLUGIN_DIR@/input

libbeatforcemp3mad_la_LDFLAGS = -module -avoid-version

libbeatforcemp3mad_la_SOURCES = mp3.c mp3.h osa.c osa.h debug.c debug.h llist.c llist.h 
INCLUDES =  -I../../../src -I$(top_builddir)/src $(MADDIR)
LIBS = -lmad -lid3tag 
