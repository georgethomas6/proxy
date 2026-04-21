#include "network_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    return rc; // getaddrinfo failed
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
 * returns positive fd upon success, -1 on error;
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
    return rc;
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
