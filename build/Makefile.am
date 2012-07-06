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

man1_MANS     = doc/pumpd.man doc/suc.man

bin_PROGRAMS  = suc pumpd

suc_SOURCES   =  src/suc/suc.c                \
                 src/suc/meta.c               \
                 src/suc/parse.c              \
                 src/suc/regex.c              \
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


pumpd_SOURCES = src/pumpd/pumpd.c            \
                src/pumpd/pumps.c            \
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



