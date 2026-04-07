CC = gcc
CFLAGS = -Wall -Wextra

all: server_threaded server_epoll

server_threaded: server_threaded.c
	$(CC) $(CFLAGS) -pthread -o server_threaded server_threaded.c

server_epoll: server_epoll.c
	$(CC) $(CFLAGS) -o server_epoll server_epoll.c

clean:
	rm -f server_threaded server_epoll main
