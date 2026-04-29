
#include "chunked_string.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 * This function initializes a chunked_string on the heap. It also initializes a
 * head, tail node. Returns NULL on malloc error, ptr to initialized
 * chunked_string on success. Before program exit user must call nuke to free
 * memory.
 * */
struct chunked_string *init_chunked_string(void) {
  struct chunked_string *chunk_str = malloc(sizeof(struct chunked_string));
  struct Node *head = malloc(sizeof(struct Node));
  if (!chunk_str || !head) {
    return NULL;
  }
  chunk_str->head = head;
  head->next = NULL;
  chunk_str->length = 0;
  head->space = MAX;
  return chunk_str;
}

/*
 * This function nukes the chunked_string chunk_str.
 */
void nuke(struct chunked_string **chunk_str) {
  if (!chunk_str || !(*chunk_str)) { // make sure there is something to free
    return;
  }
  struct Node *curr = (*chunk_str)->head;
  struct Node *tmp = NULL;
  (*chunk_str) = NULL;
  while (curr) {
    tmp = curr->next;
    free(curr);
    curr = tmp;
  }
  free(*chunk_str);
}

/*
 * This function appends buffer to the chunked_string chunk_str. Returns -1 on
 * error, 0 on success.
 * */
int add(struct chunked_string *chunk_str, char *buffer, size_t size) {

  if (!chunk_str || !buffer || !chunk_str->head) {
    return -1;
  }
  struct Node *curr;
  for (curr = chunk_str->head; curr->next && curr->space == 0;
       curr = curr->next)
    ;

  if (curr && curr->space > 0) { // fill current chunk before moving on
    size_t idx = MAX - curr->space;
    size_t min = curr->space >= size ? size : curr->space;
    assert(idx >= 0 && idx < MAX);
    memcpy(curr->chunk + idx, buffer, min);
    size -= min;
    buffer += min;
    chunk_str->length += min;
    curr->space = 0;
  }

  while (size > 0) {
    struct Node *next = malloc(sizeof(struct Node));
    if (!next) {
      return -1;
    }
    curr->next = next;
    next->space = MAX;
    next->next = NULL;
    size_t min = size >= MAX ? MAX : size;
    memcpy(next->chunk, buffer, min);
    chunk_str->length += min;
    size -= min;
    buffer += min;
  }

  return 0;
}

char *read(struct chunked_string *chunk_str) { return NULL; }
