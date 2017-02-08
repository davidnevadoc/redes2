SOURCE_LIST=src/holamundo.c main.c
OBJECT_FILES=obj/holamundo.o main.o
EXEC_NAME=holamundo
LDFLAGS=-lm
all:
	gcc -c $(OBJECT_FILES) $(SOURCE_FILES)
clean:
	rm obj/*
