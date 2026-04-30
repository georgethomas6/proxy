
#include "chunked_string.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_init_add_and_nuke_chunk_str(void);
int test_init_add_and_nuke_safe_queue(void);
int test_read(void);
void *thread(void *);

int main(int argc, char *argv[]) {

  printf("Init, add, nuke, test for chunk_str: %s\n",
         test_init_add_and_nuke_chunk_str() >= 0 ? "PASSED" : "FAILED");

  printf("Init, add, nuke, test for safe_queue: %s\n",
         test_init_add_and_nuke_safe_queue() >= 0 ? "PASSED" : "FAILED");
  printf("Read test: %s\n", test_read() >= 0 ? "PASSED" : "FAILED");
  return EXIT_SUCCESS;
}

int test_init_add_and_nuke_chunk_str(void) {
  struct chunked_string *chunk_str = init_chunked_string();
  assert(read_chunk_str(chunk_str) == NULL);
  add(chunk_str, "TEST", strlen("TEST"));
  assert(chunk_str->length == 4);
  add(chunk_str, "TEST", strlen("TEST"));
  assert(chunk_str->length == 8);
  char filler[10000];
  memset(filler, 't', 10000);
  add(chunk_str, filler, 10000);
  assert(chunk_str->length == 10008);
  nuke(&chunk_str);
  assert(add(NULL, "TEST", 4) == -1);
  assert(add(chunk_str, NULL, 4) == -1);
  assert(add(chunk_str, "l", 0) == -1);
  chunk_str = init_chunked_string();
  for (int i = 0; i < 100; i++) {
    add(chunk_str, "i", 1);
  }
  nuke(&chunk_str);
  assert(add(NULL, "TEST", 4) == -1);
  assert(add(chunk_str, NULL, 4) == -1);
  return 0;
}

int test_read(void) {
  struct chunked_string *chunk_str = init_chunked_string();
  assert(add(chunk_str, "TEST", strlen("TEST")) == 0);
  assert(chunk_str->length == 4);
  assert(chunk_str->head->next == NULL);
  assert(chunk_str->head->space == MAX - 4);

  char filler[10000];
  for (int i = 0; i < 10000; i++) {
    filler[i] = 't';
  }
  assert(add(chunk_str, filler, 10000) == 0);
  assert(chunk_str->length == 10004);
  assert(chunk_str->head->space == 0);
  assert(chunk_str->head->next->space == MAX - (10004 - MAX));
  assert(chunk_str->head->next->next == NULL);
  char *read = read_chunk_str(chunk_str);
  if (read) {
    assert(strncmp(read, "TEST", 4) == 0);
    for (int i = 0; i < 10000; i++) {
      assert(read[i + 4] == 't');
    }
    assert(read[chunk_str->length] == '\0');
    free(read);
  } else {
    return -1;
  }
  nuke(&chunk_str);
  chunk_str = init_chunked_string();
  char fill[8192];
  memset(fill, 't', 8192);
  assert(add(chunk_str, fill, 8192) == 0);
  assert(chunk_str->length == 8192);
  assert(chunk_str->head->next == NULL);
  assert(add(chunk_str, "t", 1) == 0);
  assert(chunk_str->length == 8193);
  assert(chunk_str->head->next != NULL);
  assert(chunk_str->head->next->space == 8191);
  return 0;
}

struct args {
  struct safe_queue *q;
  int i;
};
