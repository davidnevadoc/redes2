CC=gcc
FLAGS= -std=c11 -Wall -pedantic -pthread
UDPECHO=ueserv
TCPECHO=teserv

SDIR=src
ODIR=obj
DDIR=inlude

_DEPS= udpechoserver.h tcpechoserver.h
DEPS = $(patsubst %,$(DDIR)/%,$(_DEPS))

#UDP echo server 
_UESOURCES = udpechoserver.c 
UESOURCES = $(patsubst %,$(SDIR)/%,$(_SOURCES))

_UEOBJS = $(patsubst %.c, %.o, $(_UESOURCES))
UEOBJS = $(patsubst %,$(ODIR)/%,$(_UEOBJS))

#TCP echo server
_TESOURCES = tcpechoserver.c
TESOURCES = $(patsubst %,$(SDIR)/%,$(_SOURCES))

_TEOBJS = $(patsubst %.c, %.o, $(_TESOURCES))
TEOBJS = $(patsubst %,$(ODIR)/%,$(_TEOBJS))

OBJS= $(UEOBJS) $(TEOBJS)


all: $(UDPECHO) $(TCPECHO)
	@echo "#--------------------------"
	@echo " Redes 2"
	@echo " David Nevado"
	@echo " Maria Prieto"
	@echo "#--------------------------"

$(UDPECHO): $(UEOBJS)
	$(CC) $(FLAGS) -o $@ $(UEOBJS)

$(TCPECHO): $(TEOBJS)
	$(CC) $(FLAGS) -o $@ $(TEOBJS)

$(OBJS) : $(ODIR)/%.o: $(SDIR)/%.c 
	$(CC) $(FLAGS) -c -c -o $@ $<


.PHONY: clean
clean:
	rm -f $(OBJS) $(UDPECHO) $(TCPECHO) 
