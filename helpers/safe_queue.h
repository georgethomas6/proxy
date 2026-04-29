#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
struct safe_queue_Node {
  struct safe_queue_Node *next;
  int fd;
};

struct safe_queue {
  struct safe_queue_Node *head;
  struct safe_queue_Node *tail;
  sem_t *s;
  pthread_mutex_t lock;
  char *name;
};

int pop(struct safe_queue *, int *);              // pop off list
int push(struct safe_queue *, int);               // add a fd to the safe_queue
struct safe_queue *init_safe_queue(const char *); // initialize
void nuke_safe_queue(struct safe_queue **);       // teardown

#endif
