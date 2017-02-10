CC=gcc
FLAGS= -Wall -pedantic
EXE=udpechoserver

SDIR=src
ODIR=obj
DDIR=inlude

_DEPS= udpechoserver.h
DEPS = $(patsubst %,$(DDIR)/%,$(_DEPS))

_SOURCES = udpechoserver.c
SOURCES = $(patsubst %,$(SDIR)/%,$(_SOURCES))

_OBJS = $(patsubst %.c, %.o, $(_SOURCES))
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(FLAGS) -o $@ $(OBJS)

$(OBJS) : $(ODIR)/%.o: $(SDIR)/%.c 
	$(CC) $(FLAGS) -c -c -o $@ $<


.PHONY: clean
clean:
	rm -f $(OBJS) $(EXE)
