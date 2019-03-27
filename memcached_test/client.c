#include <libmemcached/memcached.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define VALUES 100
#define SIZE 4000000

/* Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1) {
  long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
  result->tv_sec = diff / 1000000;
  result->tv_usec = diff % 1000000;

  return (diff < 0);
}


void getAndProcessValue(char *key, memcached_st *memc) {
  char *received_value;
  size_t value_length;
  uint32_t flags;
  memcached_return rc;
  received_value = memcached_get(memc, key, strlen(key), &value_length, &flags, &rc);

  if(rc == MEMCACHED_SUCCESS) {
    strlen(received_value);
    free(received_value);
  }
  else
    fprintf(stderr, "Could not retreive key: %s\n", key);
}

struct timeval getFromMemcached() {
  memcached_server_st *servers = NULL;
  memcached_st *memc;
  memcached_return rc;
  struct timeval stop, start, diff;

  char key[32];
  int i, j;

  memc = memcached_create(NULL);
  servers = memcached_server_list_append(servers, "localhost", 12345, &rc);
  rc = memcached_server_push(memc, servers);
  if(rc == MEMCACHED_SUCCESS) {
    fprintf(stderr, "Added server successfully\n");
  } else {
    fprintf(stderr, "Could not add server: %s\n",
            memcached_strerror(memc, rc));
  }

  gettimeofday(&start, NULL);
  for(j = 0; j < 10; j++) {
    for(i = 0; i < VALUES; i++) {
      key[0] = '\0';
      sprintf(key, "value%d", i);
      getAndProcessValue(key, memc);
    }
  }
  gettimeofday(&stop, NULL);
  timeval_subtract(&diff, &stop, &start);

  return diff;
}

void mapAndProcessValue(char *key) {
  int i;
  int fd;
  char *map;
  fd = open(key, O_RDONLY);
  if (fd == -1) {
    perror("Error opening file for reading");
    exit(1);
  }

  map = mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(2);
  }

  //strlen(map);
  for(i = 0; map[i] != '\0'; i++);

  if (munmap(map, SIZE) == -1) {
    perror("Error un-mmapping the file");
  }
  close(fd);
}

struct timeval getFromTmp() {
  struct timeval stop, start, diff;
  char key[32];
  int i, j;
  gettimeofday(&start, NULL);
  for(j = 0; j < 10; j++) {
    for(i = 0; i < VALUES; i++) {
      key[0] = '\0';
      sprintf(key, "/tmp/value%d", i);
      mapAndProcessValue(key);
    }
  }
  gettimeofday(&stop, NULL);
  timeval_subtract(&diff, &stop, &start);
  return diff;
}

int main(int argc, char **argv) {
  struct timeval diff;
  diff = getFromMemcached();
  printf("Took %f ms", diff.tv_sec*1000.f + diff.tv_usec/1000.f);
  printf(" to get and process values from memcached\n");
  diff = getFromTmp();
  printf("Took %f ms", diff.tv_sec*1000.f + diff.tv_usec/1000.f);
  printf(" to get and process values from tmp\n");
  return 0;
}
