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
audiocd.c                       \
player.c        player.h        \
sampler.c       sample.h        \
debug.c         debug.h         \
theme.c         theme.h         \
types.h                         \
input_plugin.h                  \
output_plugin.h                 \
effect_plugin.h                 \
err.h                           \
interface.h                     \
                                \
wndmgr.c        wndmgr.h        \
player_ui.c     player_ui.h     \
songdb_ui.c     songdb_ui.h     \
playlist_ui.c   playlist_ui.h   \
mixer_ui.c      mixer_ui.h      \
clock.c         clock.h         \
                                \
file_window.c   file_window.h   \
main_window.c   main_window.h   \
search_window.c search_window.h

beatforce_LDADD   = -L../lib/src/ -lSDLTk -lSDL -lSDL_image -lfftw3 -lvrb -lxml2

