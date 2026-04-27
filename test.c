#include "http_parsing.h"
#include <stdio.h>
#include <string.h>

void test_case(char *, int, char *);
int test_parse_request_line(void);

int main(int argc, char *argv[]) {

  test_parse_request_line();
  return EXIT_SUCCESS;
}

int test_parse_request_line(void) {
  printf("=============TESTING: validate_http_request=============\n\n");

  // --- Valid cases ---
  test_case("GET /index.html HTTP/1.0\r\n\r\n", 1,
            "Valid request without headers");
  test_case("GET HTTP/1.0\r\n\r\n", 0, "Empty url");
  test_case("GET HTTP/1.0\r\n", 0, "Improper termination");
  test_case("gET HTTP/1.0\r\n", 0, "Bad method");
  test_case("", 0, "Empty request");
  test_case("\r\n\r\n", 0, "Only \\r\\n\\r\\n");
  return 1;
}

void test_case(char *line, int expected, char *description) {
  char *request = strdup(line);
  char *desc = strdup(description);

  char *result = parse_request_line(request, strlen(request)) == expected
                     ? "PASSED"
                     : "FAILED";
  printf("CASE: %s\nRESULT: %s\n\n", desc, result);
  free(request);
  free(desc);
}
