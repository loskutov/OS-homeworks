#ifndef LIB_BUFIO_H
#define LIB_BUFIO_H
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    size_t capacity;
    size_t size;
    char* data;
}buf_t;

buf_t *buf_new(size_t capacity);
void buf_free(buf_t *);
size_t buf_capacity(buf_t *);
size_t buf_size(buf_t *);
ssize_t buf_fill(int fd, buf_t* buf, size_t required);
ssize_t buf_flush(int fd, buf_t* buf, size_t required);
#endif
