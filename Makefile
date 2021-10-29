CC = clang++

INCLUDE =

CLNTS = TCPdaytime
SERVS = TCPdaytimed

CFLAGS = ${DEFS} ${INCLUDE}

CSRC = TCPdaytime.cpp 
CXSRC = connectTCP.cpp connectsock.cpp errexit.cpp

SSRC = TCPdaytimed.cpp
SXSRC = passiveTCP.cpp passivesock.cpp

SHLSRC = shell.cpp Pipe_block.cpp Command.cpp Pipe_IO.cpp Pipeline.cpp

CXOBJ = connectTCP.o connectsock.o errexit.o
SXOBJ = passiveTCP.o passivesock.o errexit.o shell.o Pipe_block.o Command.o Pipe_IO.o Pipeline.o

PROGS = ${CLNTS} ${SERVS} ${SHL}

all: ${PROGS}

${CLNTS}: ${CXOBJ}
	${CC} -o $@ ${CFLAGS} $@.o ${CXOBJ}

${SERVS}: ${SXOBJ}
	${CC} -o $@ ${CFLAGS} $@.o ${SXOBJ}  

${SHL}: ${SHLOBJ} 
	${CC} -o $@ ${CFLAGS} $@.o ${SHLOBJ}

precompile: ./cmds/noop.cpp ./cmds/number.cpp ./cmds/removetag.cpp ./cmds/removetag0.cpp
	cp /usr/bin/ls ./bin
	cp /usr/bin/cat ./bin
	$(CXX) ./cmds/noop.cpp -o ./bin/noop
	$(CXX) ./cmds/number.cpp -o ./bin/number
	$(CXX) ./cmds/removetag.cpp -o ./bin/removetag
	$(CXX) ./cmds/removetag0.cpp -o ./bin/removetag0


clients: ${CLNTS}
servers: ${SERVS}

clean: FRC
	rm -f Makefile.bak a.out core errs lint.errs ${PROGS} *.o

depend: ${HDR} ${CSRC} ${SSRC} ${TNSRC} FRC
	maketd -a ${DEFS} ${INCLUDE} ${CSRC} ${SSRC} ${TNSRC}

install: all FRC
	@echo "Your installation instructions here."

lint: ${HDR} ${XSRC} ${CSRC} ${SSRC} FRC
	lint ${DEFS} ${INCLUDE} ${CSRC} ${SSRC} ${CXSRC} ${SXSRC}

print: Makefile ${SRC} FRC
	lpr Makefile ${CSRC} ${SSRC} ${CXSRC} ${SXSRC}

spotless: clean FRC
	rcsclean Makefile ${HDR} ${SRC}

tags: ${CSRC} ${SSRC} ${CXSRC} ${SXSRC}
	ctags ${CSRC} ${SSRC} ${CXSRC} ${SXSRC}

${HDR} ${CSRC} ${CXSRC} ${SSRC} ${SXSRC}:
	co $@

TCPdaytime: TCPdaytime.o
TCPdaytimed: TCPdaytimed.o

FRC:
	
# DO NOT DELETE THIS LINE - maketd DEPENDS ON IT
S=/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/sys
I=/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include

TCPdaytime.o: $I/stdio.h TCPdaytime.cpp

TCPdaytimed.o: $I/netinet/in.h $I/stdio.h $S/types.h TCPdaytimed.cpp
