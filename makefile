CC = gcc
CFLAGS = -Wall -Wextra -std=c11
OBJ = ndn_functions.o ndn.o
EXEC = ndn_exec

all: $(EXEC)

%.o: %.c ndn_headers.h
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC)

clean:
	rm -f *.o $(EXEC)
