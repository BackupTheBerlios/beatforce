lib_LTLIBRARIES = libbeatforcecdda.la

libdir = @PLUGIN_DIR@/input

libbeatforcecdda_la_LDFLAGS = -module -avoid-version

libbeatforcecdda_la_SOURCES = cdda.c cdda.h cddb.c http.c http.h osa.c osa.h debug.c debug.h llist.c llist.h 
INCLUDES =  -I../../../src -I$(top_builddir)/src $(MADDIR)
LIBS = -lmad -lid3tag 
