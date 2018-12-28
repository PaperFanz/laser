CC = gcc
CFLAGS = -I $(IDIR) -lm

IDIR = ../include
LDIR = ../lib
ODIR = obj

_DEPS = laser.h str_const.h arr_const.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = laser.o convert.o parse.o calc.o
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