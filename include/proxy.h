#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define URL_MAX_SIZE 50
#define IP_MAX_SIZE 22

typedef struct ProxyAddress {
  char url[URL_MAX_SIZE];
  uint32_t ipAddress;
  uint16_t port;
} ProxyAddress;

typedef struct ProxyVector {
  int len;
  int capacity;
  ProxyAddress *data;
} ProxyVector;

ProxyAddress CreateProxy(uint32_t ipAddress, uint16_t port);
ProxyVector *CreateVector();
void Insert(ProxyVector *vector, ProxyAddress address);
void freeVector(ProxyVector *vector);
ProxyVector *GetProxiesFromFile(FILE *file);
bool CheckIsProxyAlive(ProxyAddress address);
