CC=gcc
CFLAGS = -Wall
LIBS = -lssl -lcrypto

ifdef DEBUG
CFLAGS += -g 
endif

PROG = genpasswd.out

all: $(PROG)

genpasswd.out: generic.h encrypt.h encrypt.c shafun.c update-record.c getpasswd.c main.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm $(PROG)