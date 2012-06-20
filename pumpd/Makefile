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

SOURCES=pumpd.c ../common/daemon.c ../common/error.c
OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=pumpd

all: $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(EXECUTABLE) 

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) gmon.out 
