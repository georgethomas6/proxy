

#include "sequential.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct args {
  int fd;
  struct sockaddr_storage client_addr;
};

void *handle_transaction_wrapper(void *args);

pthread_mutex_t LOG_LOCK;

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Proper usage: ./proxy <port number>");
    exit(EXIT_FAILURE);
  }

  int conn_fd, listen_fd;

  if ((listen_fd = gts_open_listenfd(argv[1])) < 0) { //
    perror("gts_open_listenfd failed");
    exit(EXIT_FAILURE);
  }
  pthread_mutex_init(&LOG_LOCK, NULL);
  struct sockaddr_storage client_addr;
  socklen_t client_len = sizeof(client_addr);

  while (1) {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    int fd = accept(listen_fd, (struct sockaddr *)&addr, &len);

    struct args *a = malloc(sizeof(struct args));
    if (!a) {
      close(fd);
      continue;
    }
    a->fd = fd;
    memcpy(&a->client_addr, &addr, sizeof(addr));
    pthread_t tid;
    pthread_create(&tid, NULL, handle_transaction_wrapper, a);
    pthread_detach(tid);
  }
  close(listen_fd);
  return EXIT_SUCCESS;
}

void *handle_transaction_wrapper(void *args) {
  struct args *a = args;

  char *log_str =
      handle_transaction_thread(a->fd, *(struct sockaddr *)&a->client_addr);

  if (log_str) {
    pthread_mutex_lock(&LOG_LOCK);
    write_log_entry(log_str, strlen(log_str));
    pthread_mutex_unlock(&LOG_LOCK);
    free(log_str);
  }
  free(a);
  return NULL;
}
