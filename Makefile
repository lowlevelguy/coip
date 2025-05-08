all: client middleman

CC=gcc
CFLAGS=-g -DDEBUG_MODE -Wall -Werror -Wpedantic -fsanitize=address

SRCDIR=src
ODIR=obj

OBJ=$(ODIR)/client.o $(ODIR)/middleman.o

$(ODIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(ODIR)/client.o
	$(CC) -o $@ $^ $(CFLAGS)

middleman: $(ODIR)/middleman.o
	$(CC) -o $@ $^ $(CFLAGS)

#server: $(ODIR)/$(SRVDIR)/server.o
#	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf $(ODIR) client middleman server *~ core

