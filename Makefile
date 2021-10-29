CC = g++

INCLUDE =

CLNTS = TCPdaytime
SERVS = TCPdaytimed
OTHER = superd 

CFLAGS = ${DEFS} ${INCLUDE}

CSRC = TCPdaytime.cpp 
CXSRC = connectTCP.cpp connectsock.cpp errexit.cpp

SSRC = TCPdaytimed.cpp
SXSRC = passiveTCP.cpp passivesock.cpp

CXOBJ = connectTCP.o connectsock.o errexit.o
SXOBJ = passiveTCP.o passivesock.o errexit.o

PROGS = ${CLNTS} ${SERVS} ${OTHER}

all: ${PROGS}

${CLNTS}: ${CXOBJ}
	${CC} -o $@ ${CFLAGS} $@.o ${CXOBJ} -lnsl -lsocket

${SERVS}: ${SXOBJ}
	${CC} -o $@ ${CFLAGS} $@.o ${SXOBJ}  -lnsl -lsocket

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
S=/usr/include/sys
I=/usr/include

TCPdaytime.o: $I/stdio.h TCPdaytime.cpp

TCPdaytimed.o: $I/netinet/in.h $I/stdio.h $S/types.h TCPdaytimed.cpp
