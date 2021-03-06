#include <libmemcached/memcached.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define VALUES 100
#define SIZE 4000000

int running = 1;
pthread_t tid;
int seconds = -1;

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

void* getFromMemcached(void *arg) {
  memcached_server_st *servers = NULL;
  memcached_st *memc;
  memcached_return rc;
  struct timeval stop, start, diff;

  char key[32];
  int i, operations = 0;
  double throughput;
  double timeinMillis;

  memc = memcached_create(NULL);
  servers = memcached_server_list_append(servers, "localhost", 12345, &rc);
  rc = memcached_server_push(memc, servers);
  if(rc == MEMCACHED_SUCCESS)
    fprintf(stderr, "Added server successfully\n");
  else
    fprintf(stderr, "Could not add server: %s\n", memcached_strerror(memc, rc));
  gettimeofday(&start, NULL);
  // run once
  if (seconds == -1) {
    for(i = 0; i < VALUES; i++) {
      key[0] = '\0';
      sprintf(key, "value%d", i);
      getAndProcessValue(key, memc);
    }
    gettimeofday(&stop, NULL);
    timeval_subtract(&diff, &stop, &start);
    timeinMillis = diff.tv_sec*1000.f + diff.tv_usec/1000.f;
    printf("Took %f ms\n", timeinMillis);
  } else {
    // run for X seconds
    while (running) {
      for(i = 0; i < VALUES; i++) {
        key[0] = '\0';
        sprintf(key, "value%d", i);
        getAndProcessValue(key, memc);
        operations++;
      }
    }
    gettimeofday(&stop, NULL);
    timeval_subtract(&diff, &stop, &start);
    timeinMillis = diff.tv_sec*1000.f + diff.tv_usec/1000.f;
    printf("Took %f ms\n", timeinMillis);
    printf("Operations: %d\n", operations);
    printf("Throughput: %f ops/s\n", operations/(timeinMillis/1000.f));
    printf("Average response time: %f ms\n", timeinMillis/operations);
  }
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

  for (i = 0; i < SIZE; i++)
    printf("%c\n", map[i]);

  if (munmap(map, SIZE) == -1) {
    perror("Error un-mmapping the file");
  }
  close(fd);
}

struct timeval getFromTmp() {
  struct timeval stop, start, diff;
  char key[32];
  int i;
  for(i = 0; i < VALUES; i++) {
    key[0] = '\0';
    sprintf(key, "/tmp/value%d", i);
    mapAndProcessValue(key);
  }
  return diff;
}

int main(int argc, char **argv) {
  struct timeval diff;
  int err;
  void *retval;
  if (argc > 1) {
    seconds = atoi(argv[argc - 1]);
    err = pthread_create(&(tid), NULL, &getFromMemcached, NULL);
    if (err) {
      perror("can't create thread.");
      exit(1);
    } else {
      printf("thread running.\n");
      sleep(seconds);
      running = 0;
      sleep(1);
      pthread_join(tid, &retval);
      exit(0);
    }
  }
  getFromMemcached((void *) &seconds);
  //diff = getFromTmp();
  //printf("Took %f ms\n", diff.tv_sec*1000.f + diff.tv_usec/1000.f);
  //printf(" to get and process values from tmp");
  return 0;
}
