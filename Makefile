CC = g++
CXX = clang++

# DO NOT DELETE THIS LINE - maketd DEPENDS ON IT
S=/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/sys
I=/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include
CPPFLAGS = -Wall -O2 -g -pedantic -std=c++11 -I$I/netinet/in.h -I$I/stdio.h -I$S/types.h

INCLUDE =

CLNTS = TCPdaytime
SERVS = TCPdaytimed
SERVS_1 = TCPdaytimed

CFLAGS = ${DEFS} ${INCLUDE}

CSRC = TCPdaytime.cpp 
CXSRC = connectTCP.cpp connectsock.cpp errexit.cpp

SSRC = TCPdaytimed.cpp
SXSRC = TCPdaytimed.cpp passiveTCP.cpp passivesock.cpp errexit.cpp shell.cpp Pipe_block.cpp Command.cpp Pipe_IO.cpp Pipeline.cpp

CXOBJ = TCPdaytime.o connectTCP.o connectsock.o errexit.o
SXOBJ = TCPdaytimed.o passiveTCP.o passivesock.o errexit.o shell.o Pipe_block.o Command.o Pipe_IO.o Pipeline.o

# PROGS = ${SERVS_1} TCPdaytimed

all: np_simple client

np_simple: ${SXOBJ}
	$(CXX) $(CPPFLAGS) $(SXOBJ) -o np_simple

client: ${CXOBJ}
	$(CXX) $(CPPFLAGS) $(CXOBJ) -o client\


%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

${CLNTS}: ${CXOBJ}
	${CC} -o $@ ${CFLAGS} $@.o ${CXOBJ}

${SERVS}: ${SXOBJ}
	${CC} -std=c++11 -o $@ ${CFLAGS} $@.o ${SXOBJ}  

precompile: ./cmds/noop.cpp ./cmds/number.cpp ./cmds/removetag.cpp ./cmds/removetag0.cpp
	cp /usr/bin/ls ./bin
	cp /usr/bin/cat ./bin
	$(CXX) ./cmds/noop.cpp -o ./bin/noop
	$(CXX) ./cmds/number.cpp -o ./bin/number
	$(CXX) ./cmds/removetag.cpp -o ./bin/removetag
	$(CXX) ./cmds/removetag0.cpp -o ./bin/removetag0


clients: ${CLNTS}
servers: ${SERVS}

clean:
	rm -f Makefile.bak a.out core errs ${PROGS} *.o

TCPdaytime: TCPdaytime.o
TCPdaytimed: TCPdaytimed.o


TCPdaytime.o: $I/stdio.h TCPdaytime.cpp

TCPdaytimed.o: $I/netinet/in.h $I/stdio.h $S/types.h TCPdaytimed.cpp
