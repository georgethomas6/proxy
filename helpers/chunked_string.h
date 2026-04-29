
#include <stdlib.h>

#define MAX 8192

struct Node {
  char chunk[MAX];
  size_t space;
  struct Node *next;
};

struct chunked_string {
  size_t length;     // length of string not number of nodes
  struct Node *head; // ptr to first node
};

struct chunked_string *init_chunked_string(void);
void nuke(struct chunked_string **);
struct Node *initNode(void);
int add(struct chunked_string *, char *, size_t);
char *read_chunk_str(struct chunked_string *);
