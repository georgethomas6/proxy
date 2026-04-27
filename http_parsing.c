
#include "http_parsing.h"
/*
 * Parses a request line to ensure it is correctly formatted to HTTP 1.0
 * standards
 * returns 1 if formatted OK, 0 if not
 */
int parse_request_line(char *request, size_t num_bytes) {
  char *first_space = memmem(request, num_bytes, " ", 1);
  char *last_space =
      first_space
          ? memmem(first_space + 1, num_bytes - (first_space - request), " ", 1)
          : NULL;
  if (!first_space || !last_space) {
    return 0;
  }

  int method = memcmp(request, "GET ", first_space - request) == 0;
  int non_empty_url =
      last_space - first_space + 1 != 0 &&
      !memmem(first_space + 1, last_space - first_space - 1, " ", 1);
  int version = num_bytes - (last_space - request + 1) == 12 &&
                memcmp(last_space + 1, "HTTP/1.0\r\n\r\n", 12) == 0;
  return method && non_empty_url && version;
}
