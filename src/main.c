#include "proxy.h"
#include <bits/pthreadtypes.h>
#include <curl/curl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define THREAD_COUNT 16

#define GREEN_TEXT "\033[0;32m"
#define RED_TEXT "\033[0;31m"
#define RESET_TEXT "\033[0m"

typedef struct {
  ProxyAddress address;
  int thread_id;
} ThreadArgs;

void *TestProxyThread(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  char *status = CheckIsProxyAlive(args->address)
                     ? GREEN_TEXT "Success" RESET_TEXT
                     : RED_TEXT "Failure" RESET_TEXT;
  printf("Proxy %s status: %s\n", args->address.url, status);

  free(args);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: <app> [ip_list_path]");
    return EXIT_FAILURE;
  }

  FILE *file = fopen(argv[1], "r");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  ProxyVector *list = GetProxiesFromFile(file);
  fclose(file);

  pthread_t threads[list->len];

  for (int i = 0; i < list->len; i++) {
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    args->address = list->data[i];
    args->thread_id = i;

    if (pthread_create(&threads[i], NULL, TestProxyThread, (void *)args)) {
      fprintf(stderr, "Error creating thread %d\n", i + 1);
      free(args);
    }
  }

  for (int i = 0; i < list->len; i++) {
    pthread_join(threads[i], NULL);
  }

  freeVector(list);
  return EXIT_SUCCESS;
}
