AUTOMAKE_OPTIONS = foreign subdir-objects
#                      |            \
#         gprof       non-GNU        compile objects in the 
#           |                        indicated relative paths
# optimize  | warnings  debug info
# lvl 3 \   |    /     /  
CFLAGS=-O3 -pg -Wall -g -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
LDFLAGS=-pg 
#       / 
#      /  
#  gprof  
#              
#             

man1_MANS     = doc/pumpd.man

bin_PROGRAMS  = pump pumpd

pump_SOURCES  = src/pump/pump.c          \
                src/pump/meta.c          \
                src/common/daemon.c      \
                src/common/util.c        \
                src/common/textutils.c   \
                src/common/file.c        \
                src/common/error.c       \
                src/common/sha256/sha2.c

pumpd_SOURCES = src/pumpd/pumpd.c        \
                src/common/daemon.c      \
                src/common/util.c        \
                src/common/textutils.c   \
                src/common/file.c        \
                src/common/error.c       \
                src/common/sha256/sha2.c



