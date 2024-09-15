#include "proxy.h"
#include "memory.h"
#include <curl/curl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define INITIAL_CAPACITY 10
#define TEST_URL "https://lumtest.com/myip.json"

uint8_t getByte(uint32_t ipAddress, int n) {
  int power = 24 - (n << 3);
  return (ipAddress >> power) & 0xFF;
}

ProxyAddress CreateProxy(uint32_t ipAddress, uint16_t port) {
  ProxyAddress proxy;
  proxy.ipAddress = ipAddress;
  proxy.port = port;

  uint8_t *bytes = (uint8_t *)(&ipAddress);
  sprintf(proxy.url, "http://%hhu.%hhu.%hhu.%hhu:%hu", bytes[0], bytes[1],
          bytes[2], bytes[3], port);

  return proxy;
}

ProxyVector *CreateVector() {
  ProxyVector *vector = safe_malloc(sizeof *vector);
  vector->len = 0;
  vector->capacity = INITIAL_CAPACITY;
  vector->data = safe_malloc(vector->capacity * sizeof(ProxyAddress));
  return vector;
}

void Insert(ProxyVector *vector, ProxyAddress address) {
  if (vector->len >= vector->capacity) {
    vector->capacity *= 2;
    vector->data =
        safe_realloc(vector->data, vector->capacity * sizeof(ProxyAddress));
  }
  vector->data[vector->len++] = address;
}

void freeVector(ProxyVector *vector) {
  free(vector->data);
  free(vector);
}

ProxyVector *GetProxiesFromFile(FILE *file) {

  ProxyVector *vector = CreateVector();
  char *buffer = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&buffer, &len, file)) != -1) {
    buffer[strcspn(buffer, "\n")] = 0;
    uint32_t ipAddress;
    uint16_t port;

    uint8_t *bytes = (uint8_t *)(&ipAddress);
    int n = sscanf(buffer, "%hhu.%hhu.%hhu.%hhu:%hu", &bytes[0], &bytes[1],
                   &bytes[2], &bytes[3], &port);

    if (n != 5) {
      printf("Error while parsing line {%s}.\n", buffer);
      continue;
    }

    Insert(vector, CreateProxy(ipAddress, port));
  }

  free(buffer);

  return vector;
}

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
  return size * nmemb;
}

bool CheckIsProxyAlive(ProxyAddress address) {
  CURL *curl;
  CURLcode res;
  char *url = address.url;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (!curl) {
    perror("Failed to initialize libcurl\n");
    abort();
  }

  curl_easy_setopt(curl, CURLOPT_URL, TEST_URL);
  curl_easy_setopt(curl, CURLOPT_PROXY, url);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return res == CURLE_OK;
}
