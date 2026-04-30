
#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include "helpers/http_parsing.h"
#include "helpers/network.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int handle_client_request(int, struct transaction *);
int send_client_request(int, struct transaction *);
int handle_server_response(int, int, struct transaction *);
int connect_to_server(struct transaction *, char *, int *);
int is_blocked(char *hostname);
void handle_transaction(int, struct sockaddr);
char *handle_transaction_thread(int, struct sockaddr);
void format_log_entry(char **, struct sockaddr_in *, char *, int);
void write_log_entry(char *, size_t);
int format_request(char *, char *, char **);

#endif
