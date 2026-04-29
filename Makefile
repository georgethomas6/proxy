
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

.PHONY: all

proxy: all
	$(CC)  -g -fsanitize=address $(CFLAGS) -o proxy sequential_proxy.c ./helpers/network.c ./helpers/http_parsing.c ./helpers/chunked_string.c

test: all
	$(CC)  -g -fsanitize=address,undefined $(CFLAGS) -o test ./helpers/test.c ./helpers/chunked_string.c


