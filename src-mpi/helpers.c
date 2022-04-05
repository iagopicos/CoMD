#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include "helpers.h"
#include "parallel.h"

void dump_times(array_t data, char * filename) {

  FILE *f = fopen(filename, "w");
  if (f == NULL) {
      printf("Error opening stats file!\n");
      exit(1);
  }

  fprintf(f, "#Step Time\n");
  for (unsigned int i = 0; i < kv_size(data); ++i)
    fprintf(f, "%d %f\n", i, kv_A(data,i));

  fclose(f);

}

void dump_stats(array_t data, char * filename) {

  FILE *f = fopen(filename, "w");
  if (f == NULL) {
      printf("Error opening stats file!\n");
      exit(1);
  }

  float mean = avg(data);

  fprintf(f, "#Average Max Min StdDev\n");
  fprintf(f, "%f %f %f %f\n", avg(data), max(data),  min(data), std_dev(data, mean));

  fclose(f);

}

float std_dev(array_t data, float mean) {

  float std_dev = 0.f;

  for (size_t i = 0; i < kv_size(data); ++i)
    std_dev += pow(kv_A(data, i) - mean, 2);

  return sqrt(std_dev/kv_size(data));

}

float avg(array_t data) {

  float avg = 0.f;
  for (size_t i = 0; i < kv_size(data); i++)
    avg += kv_A(data, i);

  return avg/kv_size(data);

}

float max(array_t data) {

  float max = kv_A(data, 0);
  for (size_t i = 0; i < kv_size(data); i++)
    max = max < kv_A(data, i) ? kv_A(data, i) : max;

  return max;

}

float min(array_t data) {

  float min = kv_A(data, 0);
  for (size_t i = 0; i < kv_size(data); i++)
    min = min > kv_A(data, i) ? kv_A(data, i) : min;

  return min;

}

#ifdef DO_ZMQ
void check_hostname(char * read_hostname){

  printf("Hostname: %s\n", getMyHostname());

  //TODO This is not good, it can overwrite mem
  //since read_hostname is allocated elsewhere
 // if (!strcmp(read_hostname, getMyHostname()))
    strcpy(read_hostname, "127.0.0.1");

}

char * read_hostname(char * path, char * file) {

  FILE *fp;
  char * line = NULL;
  size_t len = 0;

  char *result = malloc(strlen(path)+strlen(file)+1);//+1 for the zero-terminator
  //in real code you would check for errors in malloc here
  strcpy(result, path);
  strcat(result, file);

  fp = fopen(result, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error: Failed to open entry file - %s (please use a backslash at the end of the directory path, check if it exists and has correct permissions)\n", strerror(errno));
    return NULL;
  }

  getline(&line, &len, fp);

  fclose(fp);
  free(result);

  return line;

}

char ** read_dir(char * path, unsigned int * length) {

  DIR* FD;
  struct dirent* in_file;
  char ** dir_names = (char **) malloc(sizeof(char *));
  *length = 1;

  /* Scanning the in directory */
  if (NULL == (FD = opendir (path))) {

    fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
    return NULL;

  }

  while ((in_file = readdir(FD))) {

    if (!strcmp (in_file->d_name, "."))
      continue;
    if (!strcmp (in_file->d_name, ".."))
      continue;

    dir_names[*length - 1] = (char *) malloc((strlen(in_file->d_name)+1) * sizeof(char));
    strcpy(dir_names[*length - 1], in_file->d_name);

    char ** newpointer = realloc(dir_names, (++(*length))*sizeof(char *));
    if (newpointer == NULL) {
      return NULL;
    } else {
      dir_names = newpointer;
    }

  }

  //TODO Free stuff

  return dir_names;

}

char* rand_string_alloc(size_t size) {
  char *s = malloc(size + 1);
  const char charset[] = "0123456789";
  if (s) {
    //--size;
    for (size_t n = 0; n < size; n++) {
      int key = rand() % (int) (sizeof charset - 1);
      s[n] = charset[key];
    }
    s[size] = '\0';
  }
  return s;
}
#endif
