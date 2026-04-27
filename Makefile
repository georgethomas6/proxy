
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -o

.PHONY: all

proxy: all
	$(CC) $(CFLAGS) proxy sequential_proxy.c network_helpers.c

test:
	$(CC) -g -fsanitize=address $(CFLAGS) proxy_tests test.c http_parsing.c
