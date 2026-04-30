
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

.PHONY: all

sequential-sanitized: 
	$(CC)   -fsanitize=address $(CFLAGS) -o sequential sequential_proxy.c ./helpers/network.c ./helpers/http_parsing.c ./helpers/chunked_string.c ./sequential.c

sequential-unsanitized: 
	$(CC)   -fsanitize=address $(CFLAGS) -o sequential sequential_proxy.c ./helpers/network.c ./helpers/http_parsing.c ./helpers/chunked_string.c ./sequential.c

multi-sanitized: 
	$(CC)   -fsanitize=address $(CFLAGS) -o multi multi_proxy.c ./helpers/network.c ./helpers/http_parsing.c ./helpers/chunked_string.c ./sequential.c 

multi-unsanitized: 
	$(CC) $(CFLAGS) -o multi multi_proxy.c ./helpers/network.c ./helpers/http_parsing.c ./helpers/chunked_string.c ./sequential.c 

test: 
	$(CC) -g -fsanitize=address,undefined $(CFLAGS) -o test ./helpers/test.c ./helpers/chunked_string.c ./helpers/safe_queue.c


