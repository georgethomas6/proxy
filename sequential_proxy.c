

#include "sequential.h"

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

  while (1) {
    unsigned int client_len = sizeof(client_addr);
    conn_fd = accept(listen_fd, &client_addr, &client_len);
    // safe_queue to store conn_fds
    // pool of workers
    // waiting on safe_queue to not be empty
    // pull from safe_queue -> due the response
    handle_transaction(conn_fd, client_addr);
  }

  close(listen_fd);
  return EXIT_SUCCESS;
}
