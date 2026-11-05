# Makefile for SCTP Client-Server Practical Project (MCA Computer Networks)
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LIBS = -lsctp

TARGETS = server client server_onetoMany

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) server.c -o server $(LIBS)

client: client.c
	$(CC) $(CFLAGS) client.c -o client $(LIBS)

server_onetoMany: server_onetoMany.c
	$(CC) $(CFLAGS) server_onetoMany.c -o server_onetoMany $(LIBS)

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
