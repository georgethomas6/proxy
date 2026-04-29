
#include "chunked_string.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int test_init_add_and_nuke(void);
int test_read(void);

int main(int argc, char *argv[]) {

  printf("Init test: %s\n",
         test_init_add_and_nuke() >= 0 ? "PASSED" : "FAILED");
  return EXIT_SUCCESS;
}

int test_init_add_and_nuke(void) {
  struct chunked_string *chunk_str = init_chunked_string();
  add(chunk_str, "TEST", strlen("TEST"));
  assert(chunk_str->length == 4);
  add(chunk_str, "TEST", strlen("TEST"));
  assert(chunk_str->length == 8);
  char filler[10000];
  for (int i = 0; i < 10000; i++) {
    filler[i] = 't';
  }
  add(chunk_str, filler, 10000);
  assert(chunk_str->length == 10008);
  nuke(&chunk_str);

  return 0;
}
