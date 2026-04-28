#include "helpers/http_parsing.h"
#include "helpers/network.h"
#include <stdio.h>
#include <stdlib.h>

int handle_client_request(int, struct transaction *);
int send_client_request(int, struct transaction *);
void handle_server_reponse(int, int, struct transaction *);
int connect_to_server(struct transaction *, char *, int *);
void handle_transaction(int conn_fd);
int format_request(char *, char *, char **);
void handle_SIGPIPE(int);

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Proper usage: ./proxy <port number>");
    exit(EXIT_FAILURE);
  }

  int listen_fd, conn_fd;
  if ((listen_fd = gts_open_listenfd(argv[1])) < 0) { //
    perror("gts_open_listenfd failed");
    exit(EXIT_FAILURE);
  }
  struct sockaddr client_addr;
  signal(SIGPIPE, handle_SIGPIPE);

  while (1) {
    unsigned int client_len = sizeof(client_addr);
    conn_fd = accept(listen_fd, &client_addr, &client_len);
    handle_transaction(conn_fd);
  }

  close(listen_fd);
  return EXIT_SUCCESS;
}

/*
 * Writes the client's request to server_fd, on error it prints a message to
 * stderr and returns -1
 * On success it returns 1;
 *
 * */
int send_client_request(int server_fd, struct transaction *t) {
  // format request to send to server
  int request_len;
  // may need to make t.to_server by reference
  if ((request_len =
           format_request(t->hostname, t->pathname, &(t->to_server))) == -1) {
    perror("Failed to format request...aborting connection\n");
    return -1;
  }

  if ((t->to_server_len = socket_write(server_fd, t->to_server, request_len)) ==
      request_len) {
    return 1;
  }
  perror("Failed to write request to server...aborting connection\n");
  return -1;
}

/*
 * This function handles the server's response to the client i.e. it reads the
 * server's response and sends it to the client if successful, otherwise it
 * prints a message to stderr and returns -1
 * Returns 0 on success
 * */
void handle_server_reponse(int server_fd, int conn_fd, struct transaction *t) {

  int num_bytes;
  num_bytes = socket_read_response(server_fd, t->response);

  if (num_bytes >= 0)
    num_bytes = write(conn_fd, t->response, num_bytes);
  else
    perror("Failed to read server response\n");
}

/*
 * Formats an http GET Request to forward to the server
 * request must be free by caller
 * Returns 0 on success, -1 on error
 * */
int format_request(char *hostname, char *pathname, char **request) {
  return asprintf(request,
                  "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: "
                  "close\r\n\r\n",
                  pathname, hostname);
}

/*
 * This function attempts to connect to the server whose hostname is t->hostname
 * at port p. The hostname must be populated before function call.
 * If an error is encountered, it prints a message to stderr and returns -1.
 * Returns 0 on success
 */
int connect_to_server(struct transaction *t, char *p, int *server_fd) {
  if ((*server_fd = gts_open_clientfd(t->hostname, p)) < 0) {
    if (*server_fd == -1) {
      fprintf(stderr, "Failed to connected to %s...aborting connection\n",
              t->hostname);
    } else {
      const char *err = gai_strerror(*server_fd * -1);
      fprintf(stderr, "Failed to connect to %s: %s...aborting connection\n",
              t->hostname, err);
    }
    return -1;
  }
  return 1;
}

/*
 * This function reads the request from socket at conn_fd, validates it, and
 * parses it into fields in struct transaction t. Connection at conn_fd must be
 * established beforehand.
 * If it encounters an error, then it prints a message to stderr and returns -1
 * On success it returns 0;
 */
int handle_client_request(int conn_fd, struct transaction *t) {

  int num_bytes = socket_read_request(conn_fd, t->request);
  if (num_bytes <= 0) {
    perror("Failed to read request...aborting connection\n");
    return -1;
  }

  // validate request and put uri in uri variable
  if (validate_request(t->request, t->uri, num_bytes) == 0) {
    fprintf(stderr, "Invalid request...aborting connection\n");
    return -1;
  }

  // parse uri
  if (parse_uri(t->uri, t->hostname, t->pathname, &t->port) < 0) {
    fprintf(stderr, "Failed to parse uri...aborting connection\n");
    return -1;
  };
  return 1;
}

void handle_SIGPIPE(int signum) {
  printf("GOT SIGPIPE SHUTTING THIS HO DOWN\n");

  exit(1);
}

void handle_transaction(int conn_fd) {

  if (conn_fd == -1) {
    perror("Failed to accept connection");
    return;
  }

  struct transaction t;

  if (handle_client_request(conn_fd, &t) < 0) {
    close(conn_fd);
    return;
  };

  char *p;
  asprintf(&p, "%d", t.port);

  int server_fd;
  if (connect_to_server(&t, p, &server_fd) < 0) {
    free(p);
    close(conn_fd);
    return;
  };

  if (send_client_request(server_fd, &t) < 0) {
    free(p);
    free(t.to_server);
    close(server_fd);
    close(conn_fd);
    return;
  }

  handle_server_reponse(server_fd, conn_fd, &t);

  free(p);
  free(t.to_server);
  close(server_fd);
  close(conn_fd);
}
