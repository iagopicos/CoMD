#ifndef HELPERS_H_   /* Include guard */
#define HELPERS_H_

#define MAX_CHARS_KEY 1000
#define MAX_MESSAGE_SIZE 2048 //2 KB
#define MAX_CHARS_HOSTNAME 1024

void check_hostname(char * read_hostname);

char * read_hostname(char * path, char * file);

char ** read_dir(char * path, unsigned int * length);

char* rand_string_alloc(size_t size);

#endif // HELPERS_H_
