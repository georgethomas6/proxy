
#include "chunked_string.h"
#include <assert.h>
#include <stdio.h>
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
  if (!chunk_str) {
    return NULL;
  }
  struct Node *head = initNode();
  if (!head) {
    free(chunk_str);
    return NULL;
  }
  chunk_str->head = head;
  chunk_str->length = 0;
  return chunk_str;
}

/*
 * This function initiializes a Node on the heap. It sets node->next to NULL,
 * and node->space = MAX
 * returns the created Node on success, NULL on failure
 */
struct Node *initNode(void) {
  struct Node *node = malloc(sizeof(struct Node));
  if (!node) {
    return NULL;
  }
  node->next = NULL;
  node->space = MAX;
  memset(node->chunk, 0, MAX);
  return node;
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
  while (curr) {
    tmp = curr->next;
    free(curr);
    curr = tmp;
  }
  free(*chunk_str);
  (*chunk_str) = NULL;
}

// TODO: FIX THIS
/*
 * This function appends buffer to the chunked_string chunk_str. Returns -1 on
 * error, 0 on success.
 * */
int add(struct chunked_string *chunk_str, char *buffer, size_t size) {

  if (!chunk_str || !buffer || !chunk_str->head) {
    return -1;
  }
  struct Node *curr = NULL, *prev = NULL;
  size_t copied = 0, to_copy = 0, idx = 0;
  for (curr = chunk_str->head; curr != NULL && curr->space == 0;
       curr = curr->next) {
    prev = curr;
  }

  while (size > copied) {
    if (curr == NULL) {
      curr = initNode();
      if (!curr) {
        return -1;
      }
      prev->next = curr;
    }
    to_copy = curr->space >= size - copied ? size - copied : curr->space;
    idx = MAX - curr->space;
    assert(idx >= 0 && idx < MAX);
    memcpy(curr->chunk + idx, buffer + copied, to_copy);
    curr->space -= to_copy;
    prev = curr;
    curr = curr->next;
    copied += to_copy;
    chunk_str->length += to_copy;
  }

  return 0;
}

char *read_chunk_str(struct chunked_string *chunk_str) {
  if (!chunk_str || chunk_str->length == 0) {
    return NULL;
  }
  size_t length = chunk_str->length, copied = 0, to_copy = 0;
  char *str = malloc(chunk_str->length * sizeof(char) + 1);
  if (!str) {
    return NULL;
  }
  struct Node *curr = chunk_str->head;
  while (length > copied) {
    if (!curr) {
      fprintf(stderr, "REACHED INVALID STATE IN CHUNK_STR\n");
      nuke(&chunk_str);
      exit(EXIT_FAILURE);
    }
    to_copy = length > copied + MAX ? MAX : length - copied;
    assert(to_copy > 0 && to_copy <= MAX);
    memcpy(str + copied, curr->chunk, to_copy);
    copied += to_copy;
    curr = curr->next;
  }
  str[chunk_str->length] = '\0';
  return str;
}
