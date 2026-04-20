#include "network_helpers.h"

/*
 * Attempts to establish a listening connection
 * I tried to rewrite the open_listenfd funciton from CSAPP as an exercise
 * This is my version
 * */
int gts_open_listenfd(char *port) {

  struct addrinfo *addies, *addy, hints;
  int socket_fd, optval = 1;
  memset(&hints, 0, sizeof(struct addrinfo));
  /* makes getaddrinfo return socket addrss that can be used as endpoints for
   * connections */
  hints.ai_socktype = SOCK_STREAM;

  /* AI_PASSIVE: makes getaddrinfo return addresses that can be used by servers
   * as listening socket
   * AI_NUMERICSERV: forces the service name to be a port number */
  hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE;

  int getaddrinfo_ret;
  // On an error return the error code to main to handle there
  if ((getaddrinfo_ret = getaddrinfo(NULL, port, &hints, &addies)) != 0) {
    return getaddrinfo_ret;
  };

  addy = addies;
  do {
    if ((socket_fd = socket(addy->ai_family, addy->ai_socktype,
                            addy->ai_protocol)) == -1) {
      continue; // connection failed!
    }
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
               sizeof(int));

    if (bind(socket_fd, addy->ai_addr, addy->ai_addrlen) == 0) {
      break; // connection succeeded!
    }
    if (close(socket_fd) == -1) {
      return -1;
    };

  } while ((addy = addy->ai_next) != NULL);

  freeaddrinfo(addies);
  if (addy == NULL) {
    return -1;
  }

  // tells kernel to make socket_fd a listening socket
  // i.e. it is listening for requests
  if (listen(socket_fd, NUM_BACKLOG) != 0) {
    close(socket_fd);
    return -1;
  }
  return socket_fd;
}
