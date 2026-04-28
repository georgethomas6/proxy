#include "helpers/http_parsing.h"
#include "helpers/network.h"

/*
 * MODULARIZE:
 *
 *  1. read_request method (from client)
 *  2. read_response method (from server)
 *  3. forward request method (to server from proxy)
 *  4. serve response method (to client from proxy from server)
 */

int handle_client(int, struct transaction *);
int format_request(char *, char *, char **);
void handle_SIGPIPE(int);

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
  signal(SIGPIPE, handle_SIGPIPE);

  while (1) {
    unsigned int client_len = sizeof(client_addr);
    conn_fd = accept(listen_fd, &client_addr, &client_len);
    if (conn_fd == -1) {
      perror("Failed to accept connection");
      continue;
    }

    char request[MAXLINE], response[MAXLINE], uri[MAXLINE], hostname[MAXLINE],
        pathname[MAXLINE];
    char *to_server = NULL;
    int port = 80;

    printf("READING\n");
    int num_bytes = socket_read_request(conn_fd, request);
    if (num_bytes <= 0) {
      fprintf(stderr, "Failed to read request...aborting connection\n");
      continue;
    }

    printf("GOT %d bytes \n", num_bytes);

    print_request(request, num_bytes);

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

    num_bytes = socket_write(server_fd, to_server, request_len);
    printf("WROTE %d BYTES TO SERVER", num_bytes);

    num_bytes = socket_read_response(server_fd, response);
    printf("RESPONSE: %s", response);
    print_request(response, MAXLINE);

    if (num_bytes >= 0) {
      printf("%s", response);
      write(conn_fd, response, num_bytes);
    } else {
      fprintf(stderr, "SOMETHING FAILED MAN IDEK AT THIS POINT\n");
      continue;
    }

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

int handle_client(int conn_fd, struct transaction *t) {

  int num_bytes = socket_read_request(conn_fd, t->request);
  if (num_bytes <= 0) {
    fprintf(stderr, "Failed to read request...aborting connection\n");
    close(conn_fd);
    return -1;
  }

  // validate request and put uri in uri variable
  if (validate_request(t->request, t->uri, num_bytes) == 0) {
    fprintf(stderr, "Invalid request...aborting connection\n");
    close(conn_fd);
    return -1;
  }

  // parse uri
  if (parse_uri(t->uri, t->hostname, t->pathname, t->port) < 0) {
    fprintf(stderr, "Failed to parse uri...aborting connection\n");
    close(conn_fd);
    return -1;
  };
  return 0;
}

void handle_SIGPIPE(int signum) {
  printf("GOT SIGPIPE SHUTTING THIS HO DOWN\n");

  exit(1);
}
