CC=gcc
CFLAGS = -Wall
LIBS = -lssl -lcrypto

ifdef DEBUG
CFLAGS += -g -DDEBUG
endif

PROG = genpasswd.out
SRC = ${wildcard *.h *.c}

all: $(PROG)

$(PROG): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm $(PROG)