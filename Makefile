
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

.PHONY: all

proxy: all
	$(CC)  -g -fsanitize=address $(CFLAGS) -o proxy sequential_proxy.c ./helpers/network.c ./helpers/http_parsing.c 

test_chunked_string: all
	$(CC)  -g -fsanitize=address $(CFLAGS) -o test ./helpers/test.c ./helpers/chunked_string.c
