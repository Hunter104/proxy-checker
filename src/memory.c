#include <stdlib.h>
#include <stdio.h>
#include "memory.h"

void *safe_malloc(unsigned long size) {
  void *buf = malloc(size);
  if (!buf) {
    perror("Malloc failure");
    abort();
  }
  return buf;
}

void *safe_realloc(void *ptr, unsigned long size) {
  void *buf = realloc(ptr, size);
  if (!buf) {
    perror("Malloc failure");
    abort();
  }
  return buf;
}
