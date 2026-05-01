
#include "sequential.h"
#include "helpers/chunked_string.h"
#include "helpers/network.h"
#include <stdlib.h>
#include <string.h>

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
  char *hostname = read_chunk_str(t->hostname);
  char *pathname = read_chunk_str(t->pathname);
  if ((request_len = format_request(hostname, pathname, &(t->to_server))) ==
      -1) {
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
int handle_server_response(int server_fd, int conn_fd, struct transaction *t) {

  int num_bytes;
  num_bytes = socket_read_response(server_fd, t->response);

  char *response = read_chunk_str(t->response);
  if (num_bytes >= 0) {
    num_bytes = write(conn_fd, response, num_bytes);
    free(response);
  } else {
    perror("Failed to read server response\n");
    return -1;
  }
  return num_bytes;
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
  char *hostname = read_chunk_str(t->hostname);
  if ((*server_fd = gts_open_clientfd(hostname, p)) < 0) {
    if (*server_fd == -1) {
      fprintf(stderr, "Failed to connected to %s...aborting connection\n",
              hostname);
    } else {
      const char *err = gai_strerror(*server_fd * -1);
      fprintf(stderr, "Failed to connect to %s: %s...aborting connection\n",
              hostname, err);
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

  char *request = read_chunk_str(t->request);

  // validate request and put uri in uri variable
  if (validate_request(request, t->uri, num_bytes) == 0) {
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

/*
 * Function is the same as handle_transaction, except it returns the logstring
 * instead of logging it immediately. This way we can wrap the call to
 * write_log_entry in a mutex to ensure safety. Return value is NULL upon error.
 * Caller must free the return value.
 */
char *handle_transaction_thread(int conn_fd, struct sockaddr client_addy) {

  if (conn_fd == -1) {
    perror("Failed to accept connection");
    return NULL;
  }

  struct transaction *t = init_transaction();
  if (!t) {
    return NULL;
  }

  if (handle_client_request(conn_fd, t) < 0) {
    close(conn_fd);
    take_down_transaction(&t);
    return NULL;
  };

  char *hostname = read_chunk_str(t->hostname);
  if (is_blocked(hostname) == 0) {

    char body[] = "<!DOCTYPE html>"
                  "<html><head><title>Blocked</title></head>"
                  "<body><h1 style='font-size:60px;'>BLOCKED</h1></body>"
                  "</html>";

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.0 403 Forbidden\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "\r\n",
             strlen(body));

    socket_write(conn_fd, header, strlen(header));
    socket_write(conn_fd, body, strlen(body));

    free(hostname);
    close(conn_fd);
    take_down_transaction(&t);
    return NULL;
  }
  free(hostname);
  char *p;
  asprintf(&p, "%d", t->port);

  int server_fd;
  if (connect_to_server(t, p, &server_fd) < 0) {
    free(p);
    take_down_transaction(&t);
    close(conn_fd);
    return NULL;
  };

  if (send_client_request(server_fd, t) < 0) {
    free(p);
    free(t->to_server);
    close(server_fd);
    take_down_transaction(&t);
    close(conn_fd);
    return NULL;
  }

  int size = handle_server_response(server_fd, conn_fd, t);

  char *logstring = NULL;
  char *uri = read_chunk_str(t->uri);
  format_log_entry(&logstring, (struct sockaddr_in *)&client_addy, uri, size);

  free(p);
  free(t->to_server);
  take_down_transaction(&t);
  close(server_fd);
  close(conn_fd);
  return logstring;
}

/*
 * Function handles the transaction between the client <-> proxy <-> server
 */
void handle_transaction(int conn_fd, struct sockaddr client_addy) {

  if (conn_fd == -1) {
    perror("Failed to accept connection");
    return;
  }

  struct transaction *t = init_transaction();
  if (!t) {
    return;
  }

  if (handle_client_request(conn_fd, t) < 0) {
    close(conn_fd);
    take_down_transaction(&t);
    return;
  };

  char *hostname = read_chunk_str(t->hostname);
  if (is_blocked(hostname) == 0) {

    char body[] = "<!DOCTYPE html>"
                  "<html><head><title>Blocked</title></head>"
                  "<body><h1 style='font-size:60px;'>BLOCKED</h1></body>"
                  "</html>";

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.0 403 Forbidden\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "\r\n",
             strlen(body));

    socket_write(conn_fd, header, strlen(header));
    socket_write(conn_fd, body, strlen(body));

    free(hostname);
    close(conn_fd);
    take_down_transaction(&t);
    return;
  }
  free(hostname);
  char *p;
  asprintf(&p, "%d", t->port);

  int server_fd;
  if (connect_to_server(t, p, &server_fd) < 0) {
    free(p);
    take_down_transaction(&t);
    close(conn_fd);
    return;
  };

  if (send_client_request(server_fd, t) < 0) {
    free(p);
    free(t->to_server);
    close(server_fd);
    take_down_transaction(&t);
    close(conn_fd);
    return;
  }

  int size = handle_server_response(server_fd, conn_fd, t);

  char *logstring = NULL;
  char *uri = read_chunk_str(t->uri);
  format_log_entry(&logstring, (struct sockaddr_in *)&client_addy, uri, size);

  write_log_entry(logstring, strlen(logstring));

  free(logstring);
  free(p);
  take_down_transaction(&t);
  close(server_fd);
  close(conn_fd);
}

void write_log_entry(char *logstring, size_t len) {
  int fd = open("log.txt", O_CREAT | O_APPEND | O_WRONLY,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  write(fd, logstring, len);
  close(fd);
}

int is_blocked(char *hostname) {
  int fd = open("blocked.txt", O_RDONLY);
  if (fd < 0) {
    return 0;
  }

  char buf[MAXLINE];
  memset(buf, 0, MAXLINE);

  int n = read(fd, buf, MAXLINE - 1);
  close(fd);

  if (n <= 0) {
    return 0;
  }

  char *saveptr;
  char *line = strtok_r(buf, "\n", &saveptr);

  while (line != NULL) {
    if (strcmp(hostname, line) == 0) {
      return 0;
    }
    line = strtok_r(NULL, "\n", &saveptr);
  }

  return 1;
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char **logstring, struct sockaddr_in *sockaddr, char *uri,
                      int size) {
  time_t now;
  char time_str[MAXLINE];

  /* Get a formatted time string */
  now = time(NULL);
  strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

  char ip_in_dotted[MAXLINE];
  inet_ntop(AF_INET, &sockaddr->sin_addr, ip_in_dotted, sockaddr->sin_len);

  asprintf(logstring,
           "Time: %s\nIP ADDRESS: %s\nURI REQUESTED: %s\nSize of Server's "
           "Response: %d\n\n",
           time_str, ip_in_dotted, uri, size);
}
