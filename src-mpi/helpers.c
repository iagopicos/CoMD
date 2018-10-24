#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "helpers.h"

void check_hostname(char * read_hostname){

  char hostname[MAX_CHARS_HOSTNAME];
  hostname[MAX_CHARS_HOSTNAME - 1] = '\0';
  gethostname(hostname, MAX_CHARS_HOSTNAME - 1);
  printf("Hostname: %s\n", hostname);

  //TODO This is not good, it can overwrite mem
  //since read_hostname is allocated elsewhere
  if(!strcmp(read_hostname,hostname))
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
