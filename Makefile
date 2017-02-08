CC=gcc
FLAGS= -Wall -pedantic
EXE=exec

SDIR=src
ODIR=obj
DDIR=inlude

_DEPS=holamundo.h
DEPS = $(patsubst %,$(DDIR)/%,$(_DEPS))

_SOURCES = holamundo.c main.c
SOURCES = $(patsubst %,$(SDIR)/%,$(_SOURCES))

_OBJS = $(patsubst %.c, %.o, $(_SOURCES))
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(FLAGS) -o $@ $(OBJS)

$(OBJS) : $(ODIR)/%.o: $(SDIR)/%.c 
	$(CC) $(FLAGS) -c -c -o $@ $<



clean:
	rm obj/*
