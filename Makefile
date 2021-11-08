CXX = clang++

# DO NOT DELETE THIS LINE - maketd DEPENDS ON IT
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	S=/usr/include/sys
	I=/usr/include
	SRC=/usr/bin
endif
ifeq ($(UNAME_S),Darwin)
	S=/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/sys
	I=/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include
	SRC=/bin
endif

CPPFLAGS = -Wall -O2 -DDEBUG -g -pedantic -pthread -std=c++11 -I$I/netinet/in.h -I$I/stdio.h -I$S/types.h

CFLAGS = ${DEFS} ${INCLUDE}

DEPEND = passiveTCP.o passivesock.o errexit.o shell.o Pipe_block.o Command.o Pipe_IO.o Pipeline.o Broadcast.o BrstShrd.o semaphore.o
S1OBJ = np_simple.o ${DEPEND}
S2OBJ = np_single_proc.o ${DEPEND}
S3OBJ = np_multi_proc.o ${DEPEND}

EXE = np_simple np_single_proc np_multi_proc

all: ${EXE} precompile

np_simple: ${S1OBJ}
	$(CXX) $(CPPFLAGS) $(S1OBJ) -o $@

np_single_proc: ${S2OBJ}
	$(CXX) $(CPPFLAGS) $(S2OBJ) -o $@

np_multi_proc: ${S3OBJ}
	$(CXX) $(CPPFLAGS) $(S3OBJ) -o $@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

precompile: ./cmds/noop.cpp ./cmds/number.cpp ./cmds/removetag.cpp ./cmds/removetag0.cpp
	cp ${SRC}/ls ./bin
	cp ${SRC}/cat ./bin
	$(CXX) ./cmds/noop.cpp -o ./bin/noop
	$(CXX) ./cmds/number.cpp -o ./bin/number
	$(CXX) ./cmds/removetag.cpp -o ./bin/removetag
	$(CXX) ./cmds/removetag0.cpp -o ./bin/removetag0

clean:
	rm -f Makefile.bak ${EXE} *.o 
