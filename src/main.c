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

FILE *safe_fopen(const char *path, const char *mode) {
  FILE *file = fopen(path, mode);
  if (!file) {
    fprintf(stderr, "Failed to open file %s\n", path);
    abort();
  }
  return file;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: <app> [ip_list_path]");
    return EXIT_FAILURE;
  }

  FILE *file = safe_fopen(argv[1], "r");
  ProxyVector *list = GetProxiesFromFile(file);
  fclose(file);

#pragma omp parallel for
  for (int i = 0; i < list->len; i++) {
    char *status = CheckIsProxyAlive(list->data[i])
                       ? GREEN_TEXT "Success" RESET_TEXT
                       : RED_TEXT "Failure" RESET_TEXT;
    printf("Proxy %s status: %s\n", list->data[i].url, status);
  }

  freeVector(list);
  return EXIT_SUCCESS;
}
