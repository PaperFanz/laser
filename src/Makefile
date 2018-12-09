CC = gcc
CFLAGS = -I $(IDIR)

IDIR = ../include
LDIR = ../lib
ODIR = obj

_DEPS = laser.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = laser.o parse_args.o
OBJ = $(patsubst %, $(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

laser: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean install uninstall

clean:
	rm -f $(ODIR)/*.o

install:
	cp laser /usr/local/bin

uninstall:
	rm -f /usr/local/bin/laser