CC = g++
#CFLAGS = -O0 -g
CFLAGS = -O2

all: OPTCacheSimulator

OPTCacheSimulator: OPTCacheSimulator.o
	${CC} OPTCacheSimulator.o -o OPTCacheSimulator

OPTCacheSimulator.o: OPTCacheSimulator.cpp OPTCacheSimulator.h
	${CC} ${CFLAGS} -c OPTCacheSimulator.cpp

clean:
	rm -rf *.o OPTCacheSimulator
