
#include "http_parsing.h"
#include "network_helpers.h"

int test_parse_request_line(void);
void test_case(char *, int, char *);

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Proper usage: ./proxy <port number>");
    exit(EXIT_SUCCESS);
  }

  int listen_fd;
  if ((listen_fd = gts_open_listenfd(argv[1])) < 0) { //
    exit(EXIT_FAILURE);
  } else if (listen_fd < 0) {
    perror("gts_open_listenfd failed");
    exit(EXIT_FAILURE);
  };

  close(listen_fd);

  return EXIT_SUCCESS;
}
