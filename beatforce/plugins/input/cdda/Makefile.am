lib_LTLIBRARIES = libbeatforcecdda.la

libdir = @PLUGIN_DIR@/input

libbeatforcecdda_la_LDFLAGS = -module -avoid-version

libbeatforcecdda_la_SOURCES = cdda.c cddb.c http.c osa.c debug.c llist.c

INCLUDES =  -I../../../src -I$(top_builddir)/src $(MADDIR) -L../../../src
LIBS = -lmad -lid3tag 
