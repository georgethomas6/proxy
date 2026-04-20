#include "network_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Attempts to establish a listening connection
 * I tried to rewrite the open_listenfd funciton from CSAPP as an exercise
 * This is my version
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
    fprintf(stderr, "getaddrinfo failed in (gts_open_listenfd): %s\n",
            gai_strerror(rc));
    exit(EXIT_FAILURE);
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

  if (addy == NULL) {
    fprintf(stderr, "failed to find socket for server to listen on");
    exit(EXIT_FAILURE);
  }

  if (listen(socket_fd, NUM_BACKLOG) == -1) {
    close(socket_fd);
    perror("failed to make socket listen (gts_open_listenfd)");
    exit(EXIT_FAILURE);
  }
  return socket_fd;
}

int gts_open_clientfd(char *host, char *port) {}
