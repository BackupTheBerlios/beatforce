lib_LTLIBRARIES = libbeatforcecdda.la

libdir = @PLUGIN_DIR@/input

libbeatforcecdda_la_LDFLAGS = -module -avoid-version

libbeatforcecdda_la_SOURCES = cdda.c cdda.h cddb.c http.c http.h osa.c debug.c debug.h llist.c

INCLUDES =  -I../../../src -I$(top_builddir)/src $(MADDIR) -L../../../src
LIBS = -lmad -lid3tag 
