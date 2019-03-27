#include <libmemcached/memcached.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VALUE_SIZE 4000000
#define VALUES 100
#define ASCII_START 65
#define ASCII_END 122

char* generateRandomString() {
  int i;
  char *res = malloc(VALUE_SIZE + 1);
  for(i = 0; i < VALUE_SIZE; i++) {
    res[i] = (char) (rand()%(ASCII_END-ASCII_START))+ASCII_START;
  }
  res[i] = '\0';
  printf("%d\n", strlen(res));
  return res;
}

void createFile(char *key, char *value) {
  char *path = malloc(128);
  strcpy(path, "/tmp/");
  FILE *newFile = fopen(strcat(path, key), "w");
  free(path);
  if(!newFile) {
    perror("Error opening file");
    return;
  }
  fprintf(newFile, "%s", value);
}

void createValueMemcached(char *key, char *value, memcached_st *memc) {
  memcached_return rc;
  rc = memcached_set(memc, key, strlen(key),
                    value, strlen(value),
                    (time_t) 0, (uint32_t) 0);
  if(rc == MEMCACHED_SUCCESS)
    printf("Key stored successfully\n");
  else
    perror("Could not store key\n");
}

int main(int argc, char **argv) {
  char *value, key[32];
  int i;
  char *retrieved_value;

  memcached_server_st *servers = NULL;
  memcached_st *memc;
  memcached_return rc;

  memc = memcached_create(NULL);
  servers = memcached_server_list_append(servers, "localhost", 12345, &rc);
  rc = memcached_server_push(memc, servers);
  if(rc == MEMCACHED_SUCCESS)
    printf("Added server successfully\n");
  else
    perror("Could not add server\n");

  // set seed for random
  srand(time(NULL));
  for(i = 0; i < VALUES; i++) {
    key[0] = '\0';
    sprintf(key, "value%d", i);
    value = generateRandomString();
    createFile(key, value);
    createValueMemcached(key, value, memc);
    free(value);
  }
  return 0;
}
/*


  received_value = memcached_get(memc, key, strlen(key), &value_length, &flags, &rc);

  if(rc == MEMCACHED_SUCCESS)
    fprintf(stderr, "Key retrieved successfully\n");
    printf("The key '%s' returned value '%s'.\n", key, retrieved_value);
    free(retrieved_value);
  else
    fprintf(stderr, "Could not retreive key: %s\n", memcached_stderror(memc, rc));
*/
