
#include "http_parsing.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
/*
 * Parses a request line to ensure it is correctly formatted to HTTP 1.0
 * standards
 * returns 1 if formatted OK, 0 if not
 */
int validate_request(char *request, char *url, size_t num_bytes) {
  char *first_space = memmem(request, num_bytes, " ", 1);
  assert(num_bytes > first_space - request);
  char *last_space =
      first_space
          ? memmem(first_space + 1, num_bytes - (first_space - request), " ", 1)
          : NULL;
  if (!first_space || !last_space) {
    return 0;
  }

  int method = first_space - request == 3 &&
               memcmp(request, "GET ", first_space - request + 1) == 0;
  int non_empty_url =
      last_space - first_space + 1 != 0 &&
      !memmem(first_space + 1, last_space - first_space - 1, " ", 1);
  int version = num_bytes - (last_space - request + 1) >= 10 &&
                (memcmp(last_space + 1, "HTTP/1.1\r\n", 10) == 0 ||
                 memcmp(last_space + 1, "HTTP/1.0\r\n", 10) == 0);

  if (method && non_empty_url && version) {
    memcpy(url, first_space + 1, last_space - first_space - 1);
  } else {
    url = NULL;
  }
  return method && non_empty_url && version;
}
/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port) {
  char *hostbegin;
  char *hostend;
  char *pathbegin;
  int len;

  if (strncasecmp(uri, "http://", 7) != 0) {
    hostname[0] = '\0';
    return -1;
  }

  /* Extract the host name */
  hostbegin = uri + 7;
  hostend = strpbrk(hostbegin, " :/\r\n\0");
  len = hostend - hostbegin;
  strncpy(hostname, hostbegin, len);
  hostname[len] = '\0';

  /* Extract the port number */
  *port = 80; /* default */
  if (*hostend == ':')
    *port = atoi(hostend + 1);

  /* Extract the path */
  pathbegin = strchr(hostbegin, '/');
  if (pathbegin == NULL) {
    pathname[0] = '\0';
  } else {
    strcpy(pathname, pathbegin);
  }

  return 0;
}
