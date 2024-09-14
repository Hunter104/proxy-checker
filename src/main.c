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
#define URL_MAX_SIZE 31
#define TEST_URL "https://lumtest.com/myip.json"
#define TIMEOUT 2000

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
    ProxyAddress *address = malloc(sizeof *address);
    int n = 0;
    
    n =sscanf(buffer, "%hhu.%hhu.%hhu.%hhu:%hu", 
           &address->ipAddress[0], 
           &address->ipAddress[1],
           &address->ipAddress[2],
           &address->ipAddress[3],
           &address->port);

    if (n != 5) {
      printf("Error while parsing line {%s}.\n", buffer);
      free(address);
      continue;
    }
    
    Insert(vector, *address);
  }

  free(buffer);

  return vector;
}

char *GetUrl(ProxyAddress *address) {
  char *str = malloc(31*sizeof(char));
  sprintf(str, "http://%hhu.%hhu.%hhu.%hhu:%hu", address->ipAddress[0], address->ipAddress[1], address->ipAddress[2], address->ipAddress[3], address->port);
  
  return str;
}


typedef struct string {
  char *ptr;
  size_t len;
} string;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}

bool TestProxy(ProxyAddress *address) {
  CURL *curl;
  CURLcode res;
  char *url = GetUrl(address);

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
  if (res != CURLE_OK) {
    fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free(url);
    return false;
  }

  curl_easy_cleanup(curl);
  curl_global_cleanup();
  free(url);
  return true;
}

typedef struct {
  ProxyAddress address;
  int thread_id;
} ThreadArgs;

void *TestProxyThread(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  char *url = GetUrl(&args->address);

  printf("Thread %d: Testing proxy %s...\n", args->thread_id, url);

  if (TestProxy(&args->address)) {
    printf(GREEN_TEXT "●" RESET_TEXT "Thread %d Success!\n", args->thread_id);
  } else {
    printf(RED_TEXT "●" RESET_TEXT "Thread %d Failure\n", args->thread_id);
  }

  free(url);
  free(args);
  pthread_exit(NULL);
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

  pthread_t threads[list->len];
  for (int i=0; i<list->len; i++) {
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    args->address = list->data[i];
    args->thread_id = i + 1;

    if (pthread_create(&threads[i], NULL, TestProxyThread, (void *)args)) {
      fprintf(stderr, "Error creating thread %d\n", i + 1);
      free(args);
    }
  }

  for (int i=0; i<list->len; i++) {
    pthread_join(threads[i], NULL); 
  }

  freeVector(list);
  return EXIT_SUCCESS;
}
