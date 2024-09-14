#include <stdlib.h>
#include <stdint.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <inttypes.h>
#include "memory.h"
#include "proxy.h"
#define INITIAL_CAPACITY 10

uint8_t getByte(uint32_t ipAddress, int n) {
  int power = 24 - (n << 3);
  return (ipAddress >> power) & 0xFF;
}

ProxyAddress CreateProxy(uint32_t ipAddress, uint16_t port) {
  ProxyAddress proxy;
  proxy.ipAddress = ipAddress;
  proxy.port = port;

  uint8_t *bytes = (uint8_t*)(&ipAddress);
  sprintf(proxy.url, "http://%hhu.%hhu.%hhu.%hhu:%hu",
          bytes[0], bytes[1], bytes[2], bytes[3], port);

  return proxy;
}

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
