AUTOMAKE_OPTIONS = foreign subdir-objects
#                      |            \
#                  non-GNU        compile objects in the 
#                                 indicated relative paths
# optimize warnings debug info
# lvl 3 \     /    /  
CFLAGS=-O3 -Wall -g -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
LDFLAGS=-lpanel -lncursesw 
#           |   `````|````
#        panels    ncurses with               
#       (curses)   wide character
#                  support
#             

man1_MANS     = doc/eod.man doc/eo.man

bin_PROGRAMS  = eo eod 

eo_SOURCES = src/eo.c                     \
             src/lex.c                    \
             src/meta.c                   \
             src/parse.c                  \
             src/ops.c                    \
             src/regex.c                  \
             src/common/ipc/daemon.c      \
             src/common/ipc/channel.c     \
             src/common/ipc/fifo.c        \
             src/common/io/file.c         \
             src/common/io/dir.c          \
             src/common/io/shell.c        \
             src/common/util.c            \
             src/common/textutils.c       \
             src/common/error.c           \
             src/common/lib/bloom/bloom.c \
             src/common/lib/sha256/sha2.c


eod_SOURCES = src/eod.c                    \
              src/pumps.c                  \
              src/common/ipc/daemon.c      \
              src/common/ipc/channel.c     \
              src/common/ipc/fifo.c        \
              src/common/io/file.c         \
              src/common/io/dir.c          \
              src/common/io/shell.c        \
              src/common/util.c            \
              src/common/textutils.c       \
              src/common/error.c           \
              src/common/lib/bloom/bloom.c \
              src/common/lib/sha256/sha2.c



