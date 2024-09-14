#include <stdlib.h>
#include <stdint.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <inttypes.h>
#include "memory.h"
#include "proxy.h"
#define MAX_SIZE 256
#define INITIAL_CAPACITY 10

ProxyVector *CreateVector() {
  ProxyVector *vector = safe_malloc(sizeof *vector);
  vector->len = 0;
  vector->capacity = INITIAL_CAPACITY;
  vector->data = safe_malloc(vector->capacity*sizeof(ProxyAddress));
  return vector;
}

void Insert(ProxyVector *vector, ProxyAddress address) {
  if (vector->len >= vector->capacity) {
    vector->capacity *= 2;
    vector->data = safe_realloc(vector->data, vector->capacity*sizeof(ProxyAddress)); 
  }
  vector->data[vector->len++] = address;
}

void freeVector(ProxyVector *vector) {
  free(vector->data);
  free(vector);
}
