



CC = gcc

CFLAGS = `pkg-config --cflags gtk+-3.0`

LIBS = `pkg-config --libs gtk+-3.0` -lm

# File names

SRC = sgusers.c sgusers-uie.c

OBJ = $(SRC:.c=.o)

EXE = sgusers sgusers-uie

# Build executable files

all: $(EXE)

sgusers: sgusers.o

	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

sgusers-uie: sgusers-uie.o

	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

# Debug step

debug:

	$(CC) $(CFLAGS) -g $(SRC) -o debug $(LIBS)

# Test step

test:

	./sgusers

	./sgusers-uie

# Clean object files and executables

clean:

	rm -f $(OBJ) $(EXE) debug
