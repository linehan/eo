CC=gcc
#
#          gprof        
# optimize   |     warnings
# lvl 3 \    |     /    
CFLAGS=-O3 -pg -Wall            
LDFLAGS=-pg 
#        |
#      gprof 
#                                  

SOURCES=../common/util.c file.c ../common/error.c ../common/sha256/sha2.c pump.c
OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=pump

all: $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(EXECUTABLE) 

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) gmon.out 
