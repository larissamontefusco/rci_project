CC = gcc
CFLAGS = -Wall -Wextra -g -DDEBUG
OBJ = ndn.o ndn_functions.o
DEPS = ndn_headers.h

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

ndn: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o ndn
