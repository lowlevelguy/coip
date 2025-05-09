all: client middleman server

CC=gcc
CFLAGS=-g -DDEBUG_MODE -Wall -Werror -Wpedantic -lm
#-fsanitize=address

SRCDIR=src
ODIR=obj

_DEPS=hamming.h
DEPS=$(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ=client.o middleman.o server.o hamming.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(ODIR)/client.o $(ODIR)/hamming.o
	$(CC) -o $@ $^ $(CFLAGS)

middleman: $(ODIR)/middleman.o
	$(CC) -o $@ $^ $(CFLAGS)

server: $(ODIR)/server.o $(ODIR)/hamming.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf $(ODIR) client middleman server *~ core

