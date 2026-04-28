
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -o

.PHONY: all

proxy: all
	$(CC)  -g -fsanitize=address $(CFLAGS) proxy sequential_proxy.c ./helpers/network.c ./helpers/http_parsing.c ./helpers/csapp.c
