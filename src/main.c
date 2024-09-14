#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <inttypes.h>
#include "proxy.h"
#define URL_MAX_SIZE 31
#define TEST_URL "https://lumtest.com/myip.json"

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

  for (int i=0; i<list->len; i++) {
    ProxyAddress current = list->data[i];
    printf("%hhu.%hhu.%hhu.%hhu:%hu\n", current.ipAddress[0], current.ipAddress[1], current.ipAddress[2], current.ipAddress[3], current.port);
  }

  freeVector(list);
  return EXIT_SUCCESS;
}
