CC=gcc
#
#            
# optimize     warnings
# lvl 3 \      /    
CFLAGS=-O3 -Wall            
LDFLAGS= 
#     
#     
#                                  

SOURCES=pump.c                  \
        meta.c                  \
        ../common/daemon.c      \
        ../common/util.c        \
        ../common/textutils.c   \
        ../common/file.c        \
        ../common/error.c       \
        ../common/sha256/sha2.c

OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=pump

all: $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(EXECUTABLE) 

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) gmon.out 
