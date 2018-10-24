#ifndef HELPERS_H_   /* Include guard */
#define HELPERS_H_

#include <time.h>
#include "kvec.h"

#define MAX_CHARS_KEY 1000
#define MAX_MESSAGE_SIZE 2048 //2 KB
#define MAX_CHARS_HOSTNAME 1024

#define START_TIMING(t) clock_t t = clock()
#define END_TIMING(t) (float)(clock() - t) / CLOCKS_PER_SEC

typedef kvec_t(float) array_t;

float std_dev(array_t data, float mean);
float avg(array_t data);
float max(array_t data);
float min(array_t data);

void dump_times(array_t data, char * filename);
void dump_stats(array_t data, char * filename);

void check_hostname(char * read_hostname);

char * read_hostname(char * path, char * file);

char ** read_dir(char * path, unsigned int * length);

char * rand_string_alloc(size_t size);

#endif // HELPERS_H_
