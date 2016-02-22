CC=gcc
CFLAGS=-g -Wall -Werror -O0
LFLAGS=-lm
SRC=catc.c
EXE=Catc

all: init prog check

init:
	$(CC) $(CFLAGS) init.c -o Init $(LFLAGS)

prog:
	$(CC) $(CFLAGS) prog.c -o Prog $(LFLAGS)

seminit:
	$(CC) $(CFLAGS) seminit.c -o Seminit $(LFLAGS)

semdemo:
	$(CC) $(CFLAGS) semdemo.c -o Semdemo $(LFLAGS)

semrm:
	$(CC) $(CFLAGS) semrm.c -o Semrm $(LFLAGS)

check:
	$(CC) $(CFLAGS) check.c -o Check $(LFLAGS)
