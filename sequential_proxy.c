
#include "helpers/csapp.h"
#include "helpers/http_parsing.h"
#include "helpers/network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * MODULARIZE:
 *
 *  1. read_request method (from client)
 *  2. read_response method (from server)
 *  3. forward request method (to server from proxy)
 *  4. serve response method (to client from proxy from server)
 */

int format_request(char *, char *, char **);
void print_request(char *, size_t);

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Proper usage: ./proxy <port number>");
    exit(EXIT_SUCCESS);
  }

  int listen_fd, conn_fd;
  if ((listen_fd = gts_open_listenfd(argv[1])) < 0) { //
    perror("gts_open_listenfd failed");
    exit(EXIT_FAILURE);
  }
  struct sockaddr client_addr;

  while (1) {
    unsigned int client_len = sizeof(client_addr);
    conn_fd = accept(listen_fd, &client_addr, &client_len);
    if (conn_fd == -1) {
      perror("Failed to accept connection");
      continue;
    }

    char request[MAXLINE], reponse[MAXLINE], uri[MAXLINE], hostname[MAXLINE],
        pathname[MAXLINE];
    char *to_server = NULL;
    int port = 80;
    rio_t rio;

    // read request
    rio_readinitb(&rio, conn_fd);
    size_t num_bytes = rio_readlineb(&rio, request, MAXLINE);

    // validate request and put uri in uri variable
    if (validate_request(request, uri, num_bytes) == 0) {
      fprintf(stderr, "Invalid request...aborting connection\n");
      close(conn_fd);
      continue;
    }

    // parse uri
    if (parse_uri(uri, hostname, pathname, &port) < 0) {
      fprintf(stderr, "Failed to parse uri...aborting connection\n");
      close(conn_fd);
      continue;
    };

    // format request to send to server
    int request_len;
    if ((request_len = format_request(hostname, pathname, &to_server)) == -1) {
      fprintf(stderr, "Failed to format request...aborting connection\n");
      close(conn_fd);
      continue;
    }

    char *p;
    asprintf(&p, "%d", port);

    // fix reading!
    int server_fd;
    if ((server_fd = gts_open_clientfd(hostname, p)) < 0) {
      if (server_fd == -1) {
        fprintf(stderr, "Failed to connected to %s...aborting connection\n",
                hostname);
      } else {
        const char *err = gai_strerror(server_fd * -1);
        fprintf(stderr, "Failed to connect to %s: %s...aborting connection\n",
                hostname, err);
      }
      free(to_server);
      close(conn_fd);
      continue;
    };

    rio_writen(server_fd, to_server, request_len);

    rio_t rio2;
    rio_readinitb(&rio2, server_fd);
    while ((num_bytes = rio_readlineb(&rio2, reponse, MAXLINE)) > 0) {
      rio_writen(conn_fd, reponse, num_bytes);
      printf("%s", reponse);
    }

    rio_writen(conn_fd, "END", 3);
    // send to server and await response
    free(to_server);
    free(p);
    close(server_fd);
    close(conn_fd);
  }

  close(listen_fd);
  return EXIT_SUCCESS;
}

/*
 * Formats an http GET Request to forward to the server
 * request must be free by caller
 * Returns 0 on success, -1 on error
 * */
int format_request(char *hostname, char *pathname, char **request) {
  int ok = asprintf(request,
                    "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: "
                    "close\r\n\r\n",
                    pathname, hostname);
  return ok;
}

void print_request(char *request, size_t MAX) {
  printf("[");
  for (int i = 0; i < MAX && request[i] != '\0'; i++) {
    if (request[i] == '\r') {
      printf("\\r");
    } else if (request[i] == '\n') {
      printf("\\n");
    } else {
      printf("%c", request[i]);
    }
  }
  printf("]\n");
}
