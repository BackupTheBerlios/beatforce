rawltlibs = libraw.la
rawldflags = 

lib_LTLIBRARIES = $(rawltlibs) 

libdir = @PLUGIN_DIR@/output

libraw_la_LDFLAGS = -avoid-version -module -export-dynamic -Wall
libraw_la_SOURCES = raw.c 
INCLUDES = -I$(top_builddir)/src -I/usr/local/include


