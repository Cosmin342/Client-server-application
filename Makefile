CC = gcc
CFLAGS = -Wall -g
LIBS = -lm

all: server subscriber 

server: server.c utils.c
	$(CC) $(CFLAGS) server.c utils.c $(LIBS) -o server

subscriber: subscriber.c
	$(CC) $(CFLAGS) subscriber.c -o subscriber
clean:
	rm -f server subscriber