FLAG=-Werror
STD=-std=gnu99
CC=gcc

all:
	$(CC) $(STD) $(FLAG) client.c -o client -lpthread
	$(CC) $(STD) $(FLAG) server.c -o server -lpthread

clean:
	rm client server
