#include <unistd.h>
#include <bits/pthreadtypes.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <inttypes.h>
#include <cjson/cJSON.h>
#include <pthread.h>
#include "proxy.h"
#define TEST_URL "https://lumtest.com/myip.json"

#define GREEN_TEXT "\033[0;32m"
#define RED_TEXT "\033[0;31m"
#define RESET_TEXT "\033[0m"

ProxyVector *ParseProxies(FILE *file) {

  ProxyVector *vector = CreateVector(); 
  char *buffer = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&buffer, &len, file)) != -1) {
    buffer[strcspn(buffer, "\n")] = 0;
    uint32_t ipAddress;
    uint16_t port;

    uint8_t *bytes = (uint8_t*)(&ipAddress);
    int n = sscanf(buffer, "%hhu.%hhu.%hhu.%hhu:%hu", 
           &bytes[0], 
           &bytes[1],
           &bytes[2],
           &bytes[3],
           &port);

    if (n != 5) {
      printf("Error while parsing line {%s}.\n", buffer);
      continue;
    }
    
    Insert(vector, CreateProxy(ipAddress, port));
  }

  free(buffer);

  return vector;
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}

bool IsProxyAlive(ProxyAddress address) {
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

typedef enum { WAITING=0, FAIL, SUCCESS } ProxyState;

pthread_mutex_t lock;

typedef struct { ProxyAddress address;
  ProxyState *states;
  int thread_id;
} ThreadArgs;

void *TestProxyThread(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  bool success = IsProxyAlive(args->address);

  pthread_mutex_lock(&lock);
  args->states[args->thread_id] = success ? SUCCESS : FAIL;
  pthread_mutex_unlock(&lock);

  free(args);
  pthread_exit(NULL);
}

void PrintProxyStates(ProxyState *states, int length, bool done) {
  for (int i=0; i < length; i++) {
    pthread_mutex_lock(&lock);
    
    printf("\rProxy %d: ", i + 1);
    switch (states[i]) {
      case WAITING:
        printf(RESET_TEXT "WAITING" RESET_TEXT);
        break;
      case SUCCESS:
        printf(GREEN_TEXT "SUCCESS" RESET_TEXT);
        break;
      case FAIL:
        printf(RED_TEXT "FAILURE" RESET_TEXT);
        break;
    }
    pthread_mutex_unlock(&lock);

    printf("\n");
  }

  if (!done)
    printf("\033[%dA", length);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("Usage: <app> [ip_list_path]");
    return EXIT_FAILURE;
  }

  FILE *file = fopen(argv[1], "r");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  ProxyVector *list = ParseProxies(file);
  fclose(file);

  pthread_mutex_init(&lock, NULL);
  pthread_t threads[list->len];
  ProxyState states[list->len];

  for (int i=0; i<list->len; i++) {
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    args->address = list->data[i];
    args->thread_id = i;

    states[i] = WAITING;
    args->states = states;

    if (pthread_create(&threads[i], NULL, TestProxyThread, (void *)args)) {
      fprintf(stderr, "Error creating thread %d\n", i + 1);
      free(args);
    }
  }

  while (true) {
    bool done = true;

    for (int i=0; i < list->len; i++) {
      if (states[i] == WAITING) {
        done = false;
        break;
      }
    }

    PrintProxyStates(states, list->len, done);
    if (done)
      break;
    
    usleep(100000);
  }

  for (int i = 0; i < list->len; i++) {
    pthread_join(threads[i], NULL);
  }  

  pthread_mutex_destroy(&lock);
  freeVector(list);
  return EXIT_SUCCESS;
}
