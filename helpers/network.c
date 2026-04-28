#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 * Attempts to establish a listening connection
 * I tried to rewrite the open_listenfd funciton from CSAPP as an exercise
 * This is my version
 *
 * returns positive fd upon success, -1 or error code from getaddrinfo on error;
 * */
int gts_open_listenfd(char *port) {

  struct addrinfo *addies, *addy, hints;
  int socket_fd, optval, rc;
  optval = 1;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV; // force numeric port number
  hints.ai_flags |= AI_PASSIVE;    // makes addies point to sockets that can be
                                   // used for listening
  hints.ai_flags |=
      AI_ADDRCONFIG; // return address families that are configured on this
                     // machine (IPv4 and/or IPv6)

  if ((rc = getaddrinfo(NULL, port, &hints, &addies)) != 0) {

    fprintf(stderr, "FAILED TO ESTABLISH SERVER CONNECTION: %s\n",
            gai_strerror(rc));
    return -1; // getaddrinfo failed
  }

  for (addy = addies; addy; addy = addy->ai_next) {
    if ((socket_fd = socket(addy->ai_family, addy->ai_socktype,
                            addy->ai_protocol)) == -1) {
      continue; // failed to create socket file descriptor
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
                   sizeof(int)) == -1) {
      close(socket_fd);
      continue; // failed to make socket reusable
    }

    if (bind(socket_fd, addy->ai_addr, addy->ai_addrlen) == 0) {
      break; // successfully bound socket_fd with socket address
    }

    close(socket_fd);
  }

  freeaddrinfo(addies);

  if (addy == NULL) { // all binds failed
    return -1;
  }

  if (listen(socket_fd, NUM_BACKLOG) == -1) {
    close(socket_fd);
    return -1;
  }
  return socket_fd;
}

/*
 * Attempts to establish a connection with a server at the given port number
 * I tried to rewrite the open_clientfd funciton from CSAPP as an exercise
 * This is my version
 *
 * returns positive fd upon success, negative num on error;
 * */
int gts_open_clientfd(char *host, char *port) {

  struct addrinfo *addies, *addy, hints;
  int socket_fd, optval, rc;
  optval = 1;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV; // force numeric port number
  hints.ai_flags |=
      AI_ADDRCONFIG; // return address families that are configured on this
                     // machine (IPv4 and/or IPv6)

  if ((rc = getaddrinfo(host, port, &hints, &addies)) != 0) {
    return rc * -1;
  }
  for (addy = addies; addy; addy = addy->ai_next) {
    if ((socket_fd = socket(addy->ai_family, addy->ai_socktype,
                            addy->ai_protocol)) == -1) {
      continue; // failed to create socket file descriptor
    }

    if (connect(socket_fd, addy->ai_addr, addy->ai_addrlen) == 0) {
      break; // successfully established internet connection over socket_fd
    }

    close(socket_fd);
  }

  freeaddrinfo(addies);

  if (addy == NULL) {
    return -1; // all connections failed
  }
  return socket_fd;
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

/*
 * Reads from the file with file descriptor fd until \r\n\r\n is hit, or MAXLINE
 * bytes are read.
 * The memory for buffer must be allocated by the caller
 * returns the number of bytes read, or -1 if error
 * */
int socket_read_request(int fd, char *buffer) {

  size_t num_read = 0, tot_read = 0;

  char *pos = buffer;
  char last_four[4] = {0};

  while (memcmp(last_four, "\n\r\n\r", 4) != 0 &&
         tot_read <
             MAXLINE) { // in reverse because that is the order it is read
    if ((num_read = read(fd, pos, 1)) < 0) {
      if (errno == EINTR) { // interrupted by sig handler return
        continue;
      } else {
        perror("ERROR READING CLIENT REQUEST");
        return -1;
      }
    }
    if (num_read ==
        0) { // connection was clsoed before we finishe reading the request
      return tot_read;
    }
    pos += num_read;
    tot_read += num_read;
    if (num_read > 0) {
      for (int i = 3; i > 0; i--) {
        last_four[i] = last_four[i - 1];
      }
      last_four[0] = *(pos - 1);
    }
  }
  return tot_read;
}

int socket_write(int fd, char *buffer, size_t len) {
  int num_written = 0;
  int num_left = len;
  while (num_left > 0) {
    if ((num_written = write(fd, buffer, num_left)) == -1) {
      return -1;
    }
    num_left -= num_written;
    buffer += num_written;
  }
  return num_written;
}

/*
 * Reads from the file with file descriptor fd until EOF, or MAXLINE
 * bytes are read.
 * The memory for buffer must be allocated by the caller
 * returns the number of bytes read, or -1 if error
 * */
int socket_read_response(int fd, char *buffer) {

  size_t num_read = 0, tot_read = 0;

  char *pos = buffer;

  while (tot_read < MAXLINE) {
    if ((num_read = read(fd, pos, 1)) < 0) {
      if (errno == EINTR) { // interrupted by sig handler return
        continue;
      } else {
        perror("ERROR READING SERVER RESPONSE");
        return -1;
      }
    }
    if (num_read ==
        0) { // connection was clsoed before we finishe reading the request
      return tot_read;
    }
    pos += num_read;
    tot_read += num_read;
  }
  return tot_read;
}
