
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

.PHONY: all

sequential: all
	$(CC)   -fsanitize=address $(CFLAGS) -o sequential sequential_proxy.c ./helpers/network.c ./helpers/http_parsing.c ./helpers/chunked_string.c ./sequential.c

test: all
	$(CC)  -g -fsanitize=address,undefined $(CFLAGS) -o test ./helpers/test.c ./helpers/chunked_string.c ./helpers/safe_queue.c


