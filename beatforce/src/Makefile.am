EXTRA_DIST=res
INCLUDES = -I/usr/include/libxml2/ -I../lib/src/ -Wall 
bin_PROGRAMS = beatforce
beatforce_SOURCES =             \
main.c                          \
configfile.c    configfile.h    \
plugin.c        plugin.h        \
osa.c           osa.h           \
osa_cdrom.c     osa_cdrom.h     \
llist.c         llist.h         \
mixer.c         mixer.h         \
input.c         input.h         \
output.c        output.h        \
effect.c        effect.h        \
playlist.c      playlist.h      \
songdb.c        songdb.h        \
ringbuffer.c    ringbuffer.h    \
audio_output.c  audio_output.h  \
audiocd.c       audiocd.h       \
player.c        player.h        \
sampler.c       sampler.h       \
debug.c         debug.h         \
theme.c         theme.h         \
types.h                         \
input_plugin.h                  \
output_plugin.h                 \
effect_plugin.h                 \
err.h                           \
interface.h                     \
ladspa.h                        \
                                \
clock.c         clock.h         \
                                \
file_window.c   file_window.h   \
main_window.c   main_window.h   \
search_window.c search_window.h \
config_window.c config_window.h

beatforce_LDADD   = -L../lib/src/ -lSDLTk -lSDL -lSDL_image -lfftw3 -lxml2

