CC = gcc
#CFLAGS = -O0 -g
CFLAGS = -O2
LDFLAGS = -lm

all: OPTStackDistanceAnalyzer

OPTStackDistanceAnalyzer: OPTStackDistanceAnalyzer.o
	${CC} ${LDFLAGS} OPTStackDistanceAnalyzer.o -o OPTStackDistanceAnalyzer

OPTStackDistanceAnalyzer.o: OPTStackDistanceAnalyzer.c
	${CC} ${CFLAGS} -c OPTStackDistanceAnalyzer.c

clean:
	rm -rf *.o OPTStackDistanceAnalyzer
