#include "safe_queue.h"
#include <pthread.h>
#include <stdlib.h>

/*
 * Function intiailizes safe_queue. name must be c-string. Returns pointer to
 * initialized object on success, NULL on failure
 * */
struct safe_queue *init_safe_queue(const char *name) {
  assert(name != NULL);
  struct safe_queue *q = malloc(sizeof(struct safe_queue));
  if (!q) {
    return NULL;
  }
  q->head = NULL;
  q->tail = NULL;

  q->name = calloc(strlen(name) + 1, 1);
  if (!q->name) {
    free(q);
    return NULL;
  }

  strcpy(q->name, name);
  q->s = sem_open(q->name, O_CREAT, S_IRUSR | S_IWUSR, 0);
  if (q->s == SEM_FAILED) {
    free(q->name);
    free(q);
    return NULL;
  }
  pthread_mutex_init(&(q->lock), NULL);
  return q;
}

int pop(struct safe_queue *q, int *fd) {

  struct safe_queue_Node *tmp;
  sem_wait(q->s);
  pthread_mutex_lock(&q->lock);
  *fd = q->head->fd;
  if (q->head == q->tail) {
    free(q->head);
    q->head = NULL;
    q->tail = NULL;
  } else {
    tmp = q->head;
    q->head = q->head->next;
    free(tmp);
  }
  pthread_mutex_unlock(&q->lock);
  return 0;
}

int push(struct safe_queue *q, int fd) {

  int success = -1;
  pthread_mutex_lock(&q->lock);
  if (q->head == NULL) {
    q->head = malloc(sizeof(struct safe_queue_Node));
    if (q->head) {
      q->head->fd = fd;
      q->head->next = NULL;
      q->tail = q->head;
      success = 0;
    }
  } else {
    struct safe_queue_Node *new = malloc(sizeof(struct safe_queue_Node));
    if (new) {
      new->fd = fd;
      q->tail->next = new;
      q->tail = new;
      new->next = NULL;
      success = 0;
    }
  }
  pthread_mutex_unlock(&q->lock);
  if (success == 0) {
    sem_post(q->s);
  }
  return success;
}

/*
 * Caller must guarantee q is not in use when nuke_safe_queue is called!
 * Function frees all the memory used by safe_queue q.
 * Safe on q = NULL;
 */
void nuke_safe_queue(struct safe_queue **q) {

  if (!q || !(*q)) {
    return;
  }

  pthread_mutex_lock(&((*q)->lock));
  struct safe_queue_Node *tmp, *curr = (*q)->head;
  while (curr) {
    tmp = curr;
    curr = curr->next;
    free(tmp);
  }
  pthread_mutex_unlock(&((*q)->lock));
  pthread_mutex_destroy(&((*q)->lock));
  sem_close(((*q)->s));
  sem_unlink((*q)->name);
  free((*q)->name);
  free(*q);
  *q = NULL;
}
