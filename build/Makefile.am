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

man1_MANS     = doc/pumpd.man doc/pmp.man

bin_PROGRAMS  = pmp pumpd

pmp_SOURCES   = src/pump/pump.c              \
                src/pump/meta.c              \
                src/common/ipc/daemon.c      \
                src/common/ipc/channel.c     \
                src/common/ipc/fifo.c        \
                src/common/io/file.c         \
                src/common/io/dir.c          \
                src/common/util.c            \
                src/common/textutils.c       \
                src/common/error.c           \
                src/common/lib/bloom/bloom.c \
                src/common/sha256/sha2.c


pumpd_SOURCES = src/pumpd/pumpd.c            \
                src/pumpd/pumps.c            \
                src/common/ipc/daemon.c      \
                src/common/ipc/channel.c     \
                src/common/ipc/fifo.c        \
                src/common/io/file.c         \
                src/common/io/dir.c          \
                src/common/util.c            \
                src/common/textutils.c       \
                src/common/error.c           \
                src/common/lib/bloom/bloom.c \
                src/common/sha256/sha2.c



