
#include "network_helpers.h"

int gts_open_listenfd(char *);

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Proper usage: ./proxy <port number>");
    exit(EXIT_SUCCESS);
  }

  int listen_fd;
  if ((listen_fd = gts_open_listenfd(argv[1])) > 0) { //
    char *err = NULL;
    asprintf(&err, "failed to establish server connection (%s)",
             gai_strerror(listen_fd));
    perror(err);
    exit(EXIT_FAILURE);
  } else if (listen_fd < 0) {
    perror("gts_open_listenfd failed");
    exit(EXIT_FAILURE);
  };

  return EXIT_SUCCESS;
}
