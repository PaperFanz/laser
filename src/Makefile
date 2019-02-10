SHELL = /bin/sh

IDIR = ../include
ODIR = obj

CC = gcc
CFLAGS = -I $(IDIR)

_DEPS = laser.h config.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = laser.o assemble.o alias.o flag.o file.o label.o notify.o operand.o pseudoop.o tokenize.o macro.o offset.o
OBJ = $(patsubst %, $(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

laser: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean install uninstall

clean:
	rm -f $(ODIR)/*.o && rm -f laser

install:
	cp laser /usr/local/bin

uninstall:
	rm -f /usr/local/bin/laser