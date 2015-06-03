#ifndef LIB_HELPERS_H
#define LIB_HELPERS_H
#include <netdb.h>
#include <unistd.h>

ssize_t read_(int fd, void *buf, size_t count);
ssize_t write_(int fd, const void *buf, size_t count);

ssize_t read_until(int fd, void *buf, size_t count, char delimiter);

int spawn(const char* file, char* const argv[]);

//typedef struct {
    //char** argv;
//} execargs_t;
typedef char** execargs_t;

int exec(execargs_t* args);

execargs_t create_execargs(char * const * argv);
void free_execargs(execargs_t args);

int runpiped(execargs_t** programs, size_t n);

int connect_to_host(struct addrinfo * host);
#endif
