all: client middleman

CC=gcc
CFLAGS=-g -DDEBUG_MODE -Wall -Werror -Wpedantic -fsanitize=address

SRCDIR=src
ODIR=obj

CLIDIR=client
MDMDIR=middleman
SRVDIR=server

OBJ=$(ODIR)/$(CLIDIR)/client.o $(ODIR)/$(MDMDIR)/middleman.o


$(ODIR)/$(CLIDIR)/%.o: $(SRCDIR)/$(CLIDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/$(MDMDIR)/%.o: $(SRCDIR)/$(MDMDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/$(SRVDIR)/%.o: $(SRCDIR)/$(SRVDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<


client: $(ODIR)/$(CLIDIR)/client.o
	$(CC) -o $@ $^ $(CFLAGS)

middleman: $(ODIR)/$(MDMDIR)/middleman.o
	$(CC) -o $@ $^ $(CFLAGS)

#server: $(ODIR)/$(SRVDIR)/server.o
#	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf $(ODIR) client middleman server *~ core

