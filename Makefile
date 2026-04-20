
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -o

.PHONY: all

proxy: all
	$(CC) $(CFLAGS) proxy proxy.c network_helpers.c

