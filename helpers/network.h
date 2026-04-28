#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_BACKLOG 1024
#define MAXLINE 8096

struct transaction {
  char request[MAXLINE], response[MAXLINE], uri[MAXLINE], hostname[MAXLINE],
      pathname[MAXLINE], *to_server;
  int port;
  int to_server_len;
};
int gts_open_listenfd(char *);
int gts_open_clientfd(char *, char *);
int socket_read_request(int, char *);
int socket_read_response(int, char *);
void print_request(char *, size_t);
int socket_write(int, char *, size_t);
