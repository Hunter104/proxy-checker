#pragma once
#include <stdbool.h>
#include <inttypes.h>

typedef struct ProxyAddress {
  bool alive;
  uint8_t ipAddress[4];
  uint16_t port;
} ProxyAddress;

typedef struct ProxyVector {
  int len;
  int capacity;
  ProxyAddress *data;
} ProxyVector;

ProxyVector *CreateVector();
void Insert(ProxyVector *vector, ProxyAddress address);
void freeVector(ProxyVector *vector);
